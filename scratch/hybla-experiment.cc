#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/error-model.h"
#include "ns3/internet-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/traffic-control-module.h"

#include <cmath>
#include <cstdint>
#include <iostream>
#include <string>
#include <vector>

using namespace ns3;

static void
CountWirelessDrop(uint64_t* count, Ptr<const Packet>)
{
    ++(*count);
}

int
main(int argc, char* argv[])
{
    constexpr uint32_t flowCount = 6;
    constexpr uint16_t port = 5000;
    double targetRttMs = 100.0;
    double duration = 30.0;
    std::string foregroundTcp = "TcpHybla";
    bool sack = true;
    bool background = true;
    double per = 0.0;

    CommandLine cmd(__FILE__);
    cmd.AddValue("rtt", "Foreground propagation RTT in milliseconds", targetRttMs);
    cmd.AddValue("tcp", "Foreground TCP: TcpHybla or TcpNewReno", foregroundTcp);
    cmd.AddValue("duration", "Measurement duration in seconds", duration);
    cmd.AddValue("sack", "Enable TCP selective acknowledgments", sack);
    cmd.AddValue("background", "Enable the five background TCP flows", background);
    cmd.AddValue("per", "Wireless packet error probability per direction (0 to 1)", per);
    cmd.Parse(argc, argv);

    NS_ABORT_MSG_IF(!std::isfinite(targetRttMs) || targetRttMs < 20.0,
                    "--rtt must be at least 20 ms");
    NS_ABORT_MSG_IF(!std::isfinite(duration) || duration <= 0.0,
                    "--duration must be positive");
    NS_ABORT_MSG_IF(!std::isfinite(per) || per < 0.0 || per > 1.0,
                    "--per must be between 0 and 1");
    NS_ABORT_MSG_IF(foregroundTcp != "TcpHybla" && foregroundTcp != "TcpNewReno",
                    "Use --tcp=TcpHybla or --tcp=TcpNewReno");

    Config::SetDefault("ns3::TcpHybla::RRTT", TimeValue(MilliSeconds(25)));
    Config::SetDefault("ns3::TcpSocketBase::Sack", BooleanValue(sack));
    Config::SetDefault("ns3::TcpSocket::SndBufSize", UintegerValue(2 * 1024 * 1024));
    Config::SetDefault("ns3::TcpSocket::RcvBufSize", UintegerValue(2 * 1024 * 1024));

    NodeContainer sources;
    sources.Create(flowCount);
    NodeContainer sinks;
    sinks.Create(flowCount);
    NodeContainer routers;
    routers.Create(2);
    NodeContainer gateway;
    gateway.Create(1);

    InternetStackHelper internet;
    internet.Install(sources);
    internet.Install(sinks);
    internet.Install(routers);
    internet.Install(gateway);

    PointToPointHelper link;
    link.SetDeviceAttribute("DataRate", StringValue("10Mbps"));

    std::vector<NetDeviceContainer> devices;

    // Source access links: 2.5 ms.
    link.SetChannelAttribute("Delay", TimeValue(MicroSeconds(2500)));
    for (uint32_t i = 0; i < flowCount; ++i)
    {
        devices.push_back(link.Install(sources.Get(i), routers.Get(0)));
    }

    // Shared bottleneck R1-R2: 7.5 ms.
    link.SetChannelAttribute("Delay", TimeValue(MicroSeconds(7500)));
    NetDeviceContainer bottleneckDevices = link.Install(routers.Get(0), routers.Get(1));
    devices.push_back(bottleneckDevices);

    // Background sink access links: 2.5 ms.
    link.SetChannelAttribute("Delay", TimeValue(MicroSeconds(2500)));
    for (uint32_t i = 1; i < flowCount; ++i)
    {
        devices.push_back(link.Install(routers.Get(1), sinks.Get(i)));
    }

    link.SetChannelAttribute("Delay", TimeValue(Seconds(0)));
    devices.push_back(link.Install(routers.Get(1), gateway.Get(0)));

    // The wired portion contributes 20 ms of round-trip propagation.
    const double wirelessDelayMs = (targetRttMs - 20.0) / 2.0;
    link.SetChannelAttribute("Delay", TimeValue(Seconds(wirelessDelayMs / 1000.0)));
    const NetDeviceContainer wirelessDevices = link.Install(gateway.Get(0), sinks.Get(0));
    devices.push_back(wirelessDevices);

    uint64_t wirelessDrops[2] = {};
    for (uint32_t i = 0; i < wirelessDevices.GetN(); ++i)
    {
        const bool connected = wirelessDevices.Get(i)->TraceConnectWithoutContext(
            "PhyRxDrop", MakeBoundCallback(&CountWirelessDrop, &wirelessDrops[i]));
        NS_ABORT_MSG_IF(!connected, "Could not connect wireless PhyRxDrop trace");
    }

    // Use a separate receive error model for each direction of the wireless leg.
    if (per > 0.0)
    {
        for (uint32_t i = 0; i < wirelessDevices.GetN(); ++i)
        {
            auto errorModel = CreateObject<RateErrorModel>();
            errorModel->SetAttribute("ErrorUnit", EnumValue(RateErrorModel::ERROR_UNIT_PACKET));
            errorModel->SetAttribute("ErrorRate", DoubleValue(per));
            wirelessDevices.Get(i)->SetAttribute("ReceiveErrorModel", PointerValue(errorModel));
        }
    }

    // Install queue disciplines before address assignment to avoid default queues.
    TrafficControlHelper fifo;
    fifo.SetRootQueueDisc("ns3::FifoQueueDisc");
    for (const auto& devicePair : devices)
    {
        if (devicePair.Get(0) != bottleneckDevices.Get(0))
        {
            fifo.Install(devicePair);
        }
    }

    TrafficControlHelper red;
    red.SetRootQueueDisc("ns3::RedQueueDisc",
                         "MaxSize", QueueSizeValue(QueueSize("50p")),
                         "MinTh", DoubleValue(5.0),
                         "MaxTh", DoubleValue(15.0),
                         "LinkBandwidth", DataRateValue(DataRate("10Mbps")),
                         "LinkDelay", TimeValue(MicroSeconds(7500)),
                         "UseEcn", BooleanValue(false));

    // Keep most of the bottleneck backlog in RED, above the device queue.
    for (uint32_t i = 0; i < bottleneckDevices.GetN(); ++i)
    {
        auto device = DynamicCast<PointToPointNetDevice>(bottleneckDevices.Get(i));
        device->GetQueue()->SetMaxSize(QueueSize("1p"));
    }
    QueueDiscContainer redQueues = red.Install(bottleneckDevices);

    Ipv4AddressHelper address;
    address.SetBase("10.1.0.0", "255.255.255.0");
    for (const auto& devicePair : devices)
    {
        address.Assign(devicePair);
        address.NewNetwork();
    }
    Ipv4GlobalRoutingHelper::PopulateRoutingTables();

    const uint32_t activeFlows = background ? flowCount : 1;
    std::cout << "=== TCP Hybla experiment ===\n"
              << "Total nodes: "
              << sources.GetN() + sinks.GetN() + routers.GetN() + gateway.GetN() << '\n'
              << "Total links: " << devices.size() << '\n'
              << "Target foreground RTT: " << targetRttMs << " ms\n"
              << "Wireless one-way delay: " << wirelessDelayMs << " ms\n"
              << "Wireless PER per direction: " << 100.0 * per << "%\n"
              << "Reference RTT: 25 ms\n"
              << "RED on R1-R2: " << redQueues.GetN()
              << " queue discs; min=5 max=15 limit=50 packets\n"
              << "Foreground TCP: " << foregroundTcp
              << " | background TCP: " << (background ? "TcpNewReno" : "off")
              << " | SACK: " << (sack ? "on" : "off")
              << " | duration: " << duration << " s\n"
              << "Active background flows: " << (activeFlows - 1) << '\n';

    const Time start = Seconds(1);
    const Time measurementTime = Seconds(duration);
    NS_ABORT_MSG_IF(measurementTime.IsZero(), "--duration is below the simulation time resolution");
    std::vector<Ptr<PacketSink>> tcpSinks;
    for (uint32_t i = 0; i < activeFlows; ++i)
    {
        const std::string tcpName = (i == 0) ? foregroundTcp : "TcpNewReno";
        sources.Get(i)->GetObject<TcpL4Protocol>()->SetAttribute(
            "SocketType", TypeIdValue(TypeId::LookupByName("ns3::" + tcpName)));

        // Each sink has one network interface in addition to loopback.
        const Ipv4Address sinkIp =
            sinks.Get(i)->GetObject<Ipv4>()->GetAddress(1, 0).GetLocal();
        PacketSinkHelper receiver("ns3::TcpSocketFactory",
                                  InetSocketAddress(Ipv4Address::GetAny(), port));
        ApplicationContainer receiverApps = receiver.Install(sinks.Get(i));
        receiverApps.Start(Seconds(0));
        tcpSinks.push_back(DynamicCast<PacketSink>(receiverApps.Get(0)));

        BulkSendHelper sender("ns3::TcpSocketFactory", InetSocketAddress(sinkIp, port));
        sender.SetAttribute("MaxBytes", UintegerValue(0));
        ApplicationContainer senderApps = sender.Install(sources.Get(i));
        senderApps.Start(start);
    }

    Simulator::Stop(start + measurementTime);
    Simulator::Run();

    double totalMbps = 0.0;
    for (uint32_t i = 0; i < activeFlows; ++i)
    {
        const uint64_t bytes = tcpSinks[i]->GetTotalRx();
        const double goodputMbps = 8.0 * bytes / (measurementTime.GetSeconds() * 1e6);
        totalMbps += goodputMbps;
        std::cout << (i == 0 ? "Foreground" : "Background") << '[' << i << "] rxBytes="
                  << bytes << " goodputMbps=" << goodputMbps << '\n';
    }
    std::cout << "Total goodputMbps: " << totalMbps << '\n';

    for (uint32_t i = 0; i < redQueues.GetN(); ++i)
    {
        std::cout << "RED[" << i << "] droppedPackets="
                  << redQueues.Get(i)->GetStats().nTotalDroppedPackets << '\n';
    }

    std::cout << "Wireless drops forward (gateway -> sink): " << wirelessDrops[1] << '\n'
              << "Wireless drops reverse (sink -> gateway): " << wirelessDrops[0] << '\n';

    Simulator::Destroy();
    return 0;
}
