#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/internet-module.h"
#include "ns3/lte-module.h"
#include "ns3/lorawan-module.h"
#include "ns3/netanim-module.h"
using namespace ns3;

int main(int argc, char *argv[]) {
    // Command line arguments
    CommandLine cmd;
    cmd.Parse(argc, argv);

    // Enable tracing
   // p2p.EnablePcapAll("iot-lorawan-lte");
    AnimationInterface anim("lorawan-animation.xml"); // specify the filename for NetAnim

    // Run the simulation
    Simulator::Run();
    Simulator::Destroy();

    return 0;
}

