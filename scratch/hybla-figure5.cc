#include "ns3/applications-module.h"
#include <vector>
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/internet-module.h"

#include <iostream>

using namespace ns3;

int main(int argc, char* argv[])
{
    LogComponentEnable("UdpEchoClientApplication", LOG_LEVEL_INFO);
    LogComponentEnable("UdpEchoServerApplication", LOG_LEVEL_INFO);
    
    // Configuration
    double targetRttMs = 100.0;
    CommandLine cmd(__FILE__);
    cmd.AddValue("rtt", "Foreground RTT in milliseconds", targetRttMs);
    cmd.Parse(argc, argv);

    NS_ABORT_MSG_IF(targetRttMs < 25.0 || targetRttMs > 600.0,
                    "RTT must be between 25 and 600 ms");

    Config::SetDefault("ns3::TcpHybla::RRTT",
                       TimeValue(MilliSeconds(25)));

    // Create nodes
    NodeContainer sources;
    sources.Create(6); // 1 foreground + 5 background

    NodeContainer sinks;
    sinks.Create(6);

    NodeContainer routers;
    routers.Create(2); // R1 and R2

    NodeContainer gateway;
    gateway.Create(1);

    // Install internet protocol stack
    InternetStackHelper internet;
    internet.Install(sources);
    internet.Install(sinks);
    internet.Install(routers);
    internet.Install(gateway);

    PointToPointHelper link;
    link.SetDeviceAttribute("DataRate", StringValue("10Mbps"));

    uint32_t linkCount = 0;
    std::vector<NetDeviceContainer> devices;

    // Source access links: 2.5 ms
    link.SetChannelAttribute("Delay", StringValue("2.5ms"));
    for (uint32_t i = 0; i < 6; ++i)
    {
        devices.push_back(link.Install(sources.Get(i), routers.Get(0)));
        ++linkCount;
    }

    // Shared bottleneck: R1 to R2, 7.5 ms
    link.SetChannelAttribute("Delay", StringValue("7.5ms"));
    devices.push_back(link.Install(routers.Get(0), routers.Get(1)));
    ++linkCount;

    // Background sink access links: 2.5 ms
    link.SetChannelAttribute("Delay", StringValue("2.5ms"));
    for (uint32_t i = 1; i < 6; ++i)
    {
        devices.push_back(link.Install(routers.Get(1), sinks.Get(i)));
        ++linkCount;
    }

    // R2 to wireless gateway: 0 ms
    link.SetChannelAttribute("Delay", StringValue("0ms"));
    devices.push_back(link.Install(routers.Get(1), gateway.Get(0)));
    ++linkCount;

    // Wireless leg: variable propagation delay
    double wirelessDelayMs = (targetRttMs - 20.0) / 2.0;

    link.SetChannelAttribute(
        "Delay", TimeValue(MilliSeconds(wirelessDelayMs)));

    devices.push_back(link.Install(gateway.Get(0), sinks.Get(0)));
    ++linkCount;

    // Assign IPv4 addresses
    Ipv4AddressHelper address;
    address.SetBase("10.1.0.0", "255.255.255.0");

    for (const auto& devicePair : devices)
    {
        address.Assign(devicePair);
        address.NewNetwork();
    }

    // Populate routing tables
    Ipv4GlobalRoutingHelper::PopulateRoutingTables();

    // Test end-to-end connectivity using UDP Echo
    Ipv4Address foregroundSinkAddress =
        sinks.Get(0)->GetObject<Ipv4>()->GetAddress(1, 0).GetLocal();

    uint16_t echoPort = 9;

    UdpEchoServerHelper echoServer(echoPort);

    ApplicationContainer serverApp =
        echoServer.Install(sinks.Get(0));

    serverApp.Start(Seconds(1.0));
    serverApp.Stop(Seconds(5.0));

    UdpEchoClientHelper echoClient(foregroundSinkAddress, echoPort);

    echoClient.SetAttribute("MaxPackets", UintegerValue(1));
    echoClient.SetAttribute("Interval", TimeValue(Seconds(1.0)));
    echoClient.SetAttribute("PacketSize", UintegerValue(1024));

    ApplicationContainer clientApp =
    echoClient.Install(sources.Get(0));

    clientApp.Start(Seconds(2.0));
    clientApp.Stop(Seconds(5.0));

    NS_LOG_UNCOND("Testing UDP Echo connectivity...");

    std::cout << "IPv4 addressing: completed\n";
    std::cout << "Global routing: completed\n";  

    std::cout << "=== TCP Hybla Figure 5 Topology ===\n";
    std::cout << "Foreground flows: 1\n";
    std::cout << "Background flows: 5\n";
    std::cout << "Total nodes: "
              << NodeList::GetNNodes() << "\n";
    std::cout << "Total links: " << linkCount << "\n";
    std::cout << "Bottleneck: 10 Mbps, 7.5 ms\n";
    std::cout << "Target foreground RTT: "
              << targetRttMs << " ms\n";
    std::cout << "Wireless one-way delay: "
              << wirelessDelayMs << " ms\n";
    std::cout << "Reference RTT: 25 ms\n";

    Simulator::Run();
    Simulator::Destroy();
    return 0;
}
	
