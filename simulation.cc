#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/internet-module.h"
#include "ns3/lte-module.h"
#include "ns3/lorawan-module.h"

using namespace ns3;

int main(int argc, char *argv[]) {
    // Command line arguments
    CommandLine cmd;
    cmd.Parse(argc, argv);

    // Create nodes: IoT devices, a gateway, and a server
    NodeContainer lorawanNodes;
    lorawanNodes.Create(10);

    Ptr<Node> gateway = CreateObject<Node>();
    Ptr<Node> server = CreateObject<Node>();

    // LoRaWAN setup
    LoRaWANHelper lorawanHelper;
    lorawanHelper.Install(lorawanNodes);

    // Create LoRaWAN gateway
    lorawanHelper.InstallGateway(gateway);

    // LTE setup
    NodeContainer enbNodes;
    enbNodes.Create(1);
    
    NodeContainer ueNodes;
    ueNodes.Add(gateway);

    // Install LTE modules
    LteHelper lteHelper;
    lteHelper.InstallEnbDevice(enbNodes);
    lteHelper.InstallUeDevice(ueNodes);

    // Install the IP stack on the gateway and the server
    InternetStackHelper internet;
    internet.Install(ueNodes);
    internet.Install(server);

    // Assign IP addresses
    Ipv4AddressHelper ipv4;
    ipv4.SetBase("7.0.0.0", "255.0.0.0");

    Ipv4InterfaceContainer enbIpIfaces;
    enbIpIfaces = ipv4.Assign(lteHelper.GetEnbDevice(enbNodes));

    Ipv4InterfaceContainer ueIpIfaces;
    ueIpIfaces = ipv4.Assign(lteHelper.GetUeDevice(ueNodes));

    // Connect gateway to the server via Point-to-Point link for simplicity
    PointToPointHelper p2p;
    p2p.SetDeviceAttribute("DataRate", StringValue("100Mbps"));
    p2p.SetChannelAttribute("Delay", StringValue("10ms"));

    NetDeviceContainer p2pDevices;
    p2pDevices = p2p.Install(gateway, server);

    Ipv4InterfaceContainer p2pInterfaces;
    p2pInterfaces = ipv4.Assign(p2pDevices);

    // Create Applications
    uint16_t port = 9;

    // On the server
    UdpServerHelper udpServer(port);
    ApplicationContainer serverApp = udpServer.Install(server);
    serverApp.Start(Seconds(1.0));
    serverApp.Stop(Seconds(10.0));

    // On the gateway
    UdpClientHelper udpClient(p2pInterfaces.GetAddress(1), port);
    udpClient.SetAttribute("MaxPackets", UintegerValue(320));
    udpClient.SetAttribute("Interval", TimeValue(Seconds(0.05)));
    udpClient.SetAttribute("PacketSize", UintegerValue(1024));

    ApplicationContainer clientApp = udpClient.Install(gateway);
    clientApp.Start(Seconds(2.0));
    clientApp.Stop(Seconds(10.0));

    // Enable tracing
    p2p.EnablePcapAll("iot-lorawan-lte");

    // Run the simulation
    Simulator::Run();
    Simulator::Destroy();

    return 0;
}

