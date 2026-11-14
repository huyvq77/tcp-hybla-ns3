#include "ns3/core-module.h"
#include "ns3/internet-module.h"

#include <cmath>
#include <iomanip>
#include <iostream>
#include <vector>

using namespace ns3;

// Called whenever the rho trace value changes
void
RhoUpdated(double oldVal, double newVal)
{
    std::cout << "Rho updated: "
              << oldVal << " -> " << newVal << std::endl;
}

void
SaveRho(double* result, double oldVal, double newVal)
{
    *result = newVal;
}

int
main(int argc, char* argv[])
{
    Config::SetDefault("ns3::TcpHybla::RRTT",
                       TimeValue(MilliSeconds(25)));

    std::vector<int> rtts = {25, 50, 100, 200};

    std::cout << "TCP Hybla - Rho Verification\n";
    std::cout << "Reference RTT = 25 ms\n\n";

    bool allPassed = true;

    for (int rttMs : rtts)
    {
        Ptr<TcpSocketState> state = CreateObject<TcpSocketState>();
        state->m_minRtt = MilliSeconds(rttMs);

        Ptr<TcpHybla> hybla = CreateObject<TcpHybla>();

        double actualRho = 1.0;

        // Store rho through the NS-3 TraceSource
        hybla->TraceConnectWithoutContext(
   		 "Rho",
   		 MakeBoundCallback(&SaveRho, &actualRho));

        // Trigger rho calculation
        hybla->PktsAcked(state, 1, MilliSeconds(rttMs));

        double expectedRho =
            std::max(static_cast<double>(rttMs) / 25.0, 1.0);

        bool passed = std::abs(actualRho - expectedRho) < 0.0001;
        allPassed = allPassed && passed;

        std::cout << "RTT = " << std::setw(3) << rttMs
                  << " ms | Expected rho = " << expectedRho
                  << " | Actual rho = " << actualRho
                  << " | " << (passed ? "PASS" : "FAIL")
                  << std::endl;
    }

    Simulator::Destroy();

    return allPassed ? 0 : 1;
}
