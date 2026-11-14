#include "ns3/core-module.h"
#include "ns3/internet-module.h"

#include <iostream>

using namespace ns3;

int main(int argc, char* argv[])
{
    // Set the reference RTT for TCP Hybla
    Config::SetDefault("ns3::TcpHybla::RRTT",
                       TimeValue(MilliSeconds(25)));

    // Select TCP Hybla
    Config::SetDefault("ns3::TcpL4Protocol::SocketType",
                       TypeIdValue(TcpHybla::GetTypeId()));

    // Create a TCP Hybla object for verification
    Ptr<TcpHybla> hybla = CreateObject<TcpHybla>();

    // Read the actual configured value
    TimeValue rrtt;
    hybla->GetAttribute("RRTT", rrtt);

    std::cout << "TCP variant: TcpHybla" << std::endl;
    std::cout << "Reference RTT: "
              << rrtt.Get().GetMilliSeconds()
              << " ms" << std::endl;

    Simulator::Destroy();
    return 0;
}
