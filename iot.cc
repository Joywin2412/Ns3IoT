#include "ns3/command-line.h"
#include "ns3/constant-position-mobility-model.h"
#include "ns3/end-device-lora-phy.h"
#include "ns3/end-device-lorawan-mac.h"
#include "ns3/gateway-lora-phy.h"
#include "ns3/gateway-lorawan-mac.h"
#include "ns3/log.h"
#include "ns3/lora-helper.h"
#include "ns3/mobility-helper.h"
#include "ns3/node-container.h"
#include "ns3/one-shot-sender-helper.h"
#include "ns3/position-allocator.h"
#include "ns3/simulator.h"
#include "ns3/lte-helper.h"
#include "ns3/point-to-point-epc-helper.h"
#include "ns3/internet-module.h"
#include "ns3/applications-module.h"
#include "ns3/ipv4-static-routing-helper.h"

// #include "ns3/lte-helper.h"
#include "ns3/epc-helper.h"
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/lte-module.h"
#include "ns3/applications-module.h"
#include "ns3/point-to-point-helper.h"
#include "ns3/config-store.h"
#include "ns3/netanim-module.h"
#include <algorithm>
#include <ctime>

using namespace ns3;
using namespace lorawan;
//using namespace lte;
NS_LOG_COMPONENT_DEFINE("SimpleLorawanNetworkExample");

int main(int argc, char* argv[])
{
    // Set up logging
    LogComponentEnable("SimpleLorawanNetworkExample", LOG_LEVEL_ALL);
    LogComponentEnable("LoraChannel", LOG_LEVEL_INFO);
    LogComponentEnable("LoraPhy", LOG_LEVEL_ALL);
    LogComponentEnable("EndDeviceLoraPhy", LOG_LEVEL_ALL);
    LogComponentEnable("GatewayLoraPhy", LOG_LEVEL_ALL);
    LogComponentEnable("LoraInterferenceHelper", LOG_LEVEL_ALL);
    LogComponentEnable("LorawanMac", LOG_LEVEL_ALL);
    LogComponentEnable("EndDeviceLorawanMac", LOG_LEVEL_ALL);
    LogComponentEnable("ClassAEndDeviceLorawanMac", LOG_LEVEL_ALL);
    LogComponentEnable("GatewayLorawanMac", LOG_LEVEL_ALL);
    LogComponentEnable("LogicalLoraChannelHelper", LOG_LEVEL_ALL);
    LogComponentEnable("LogicalLoraChannel", LOG_LEVEL_ALL);
    LogComponentEnable("LoraHelper", LOG_LEVEL_ALL);
    LogComponentEnable("LoraPhyHelper", LOG_LEVEL_ALL);
    LogComponentEnable("LorawanMacHelper", LOG_LEVEL_ALL);
    LogComponentEnable("OneShotSenderHelper", LOG_LEVEL_ALL);
    LogComponentEnable("OneShotSender", LOG_LEVEL_ALL);
    LogComponentEnable("LorawanMacHeader", LOG_LEVEL_ALL);
    LogComponentEnable("LoraFrameHeader", LOG_LEVEL_ALL);
    LogComponentEnableAll(LOG_PREFIX_FUNC);
    LogComponentEnableAll(LOG_PREFIX_NODE);
    LogComponentEnableAll(LOG_PREFIX_TIME);

    /************************
     *  Create the channel  *
     ************************/

    NS_LOG_INFO("Creating the channel...");

    // Create the lora channel object
    Ptr<LogDistancePropagationLossModel> loss = CreateObject<LogDistancePropagationLossModel>();
    loss->SetPathLossExponent(3.76);
    loss->SetReference(1, 7.7);

    Ptr<PropagationDelayModel> delay = CreateObject<ConstantSpeedPropagationDelayModel>();

    Ptr<LoraChannel> channel = CreateObject<LoraChannel>(loss, delay);

    /************************
     *  Create the helpers  *
     ************************/

    NS_LOG_INFO("Setting up helpers...");

    MobilityHelper mobility;
    Ptr<ListPositionAllocator> allocator = CreateObject<ListPositionAllocator>();

    // Gateway position (center)
    // allocator->Add(Vector(0, 0, 0));

    // End devices positions (circular arrangement around the gateway)
    double radius = 1000.0;  // distance from the gateway
    for (int i = 0; i < 10; ++i)
    {
        double angle = i * (2 * M_PI / 10); // 10 nodes evenly spaced in a circle
        double x = radius * cos(angle);
        double y = radius * sin(angle);
        allocator->Add(Vector(x, y, 0));
    }

    mobility.SetPositionAllocator(allocator);
    mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");

    // Create the LoraPhyHelper
    LoraPhyHelper phyHelper;
    phyHelper.SetChannel(channel);

    // Create the LorawanMacHelper
    LorawanMacHelper macHelper;

    // Create the LoraHelper
    LoraHelper helper;

    /************************
     *  Create End Devices  *
     ************************/

    NS_LOG_INFO("Creating the end devices...");

    // Create a set of nodes
    NodeContainer endDevices;
    endDevices.Create(10);

    // Assign a mobility model to the nodes
    mobility.Install(endDevices);

    // Create the LoraNetDevices of the end devices
    phyHelper.SetDeviceType(LoraPhyHelper::ED);
    macHelper.SetDeviceType(LorawanMacHelper::ED_A);
    helper.Install(phyHelper, macHelper, endDevices);

    /*********************
     *  Create Gateways  *
     *********************/

    NS_LOG_INFO("Creating the gateway...");
    NodeContainer gateways;
    gateways.Create(1);


 MobilityHelper mobility3;
     allocator = CreateObject<ListPositionAllocator>();

    // Gateway position (center)
    allocator->Add(Vector(0, 0, 0));

    // End devices positions (circular arrangement around the gateway)
   
    mobility3.SetPositionAllocator(allocator);
    mobility3.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    // Install mobility3 model for the gateway
    mobility3.Install(gateways);

    // Create a netdevice for the gateway
    phyHelper.SetDeviceType(LoraPhyHelper::GW);
    macHelper.SetDeviceType(LorawanMacHelper::GW);
    helper.Install(phyHelper, macHelper, gateways);


    // Lte config
    // 
    // 

    uint16_t numberOfNodes = 2;
  double simTime = 1.1;
  double distance = 60.0;
  double interPacketInterval = 100;

  CommandLine cmd;
  cmd.AddValue("numberOfNodes", "Number of eNodeBs + UE pairs", numberOfNodes);
  cmd.AddValue("simTime", "Total duration of the simulation [s])", simTime);
  cmd.AddValue("distance", "Distance between eNBs [m]", distance);
  cmd.AddValue("interPacketInterval", "Inter packet interval [ms])", interPacketInterval);
  cmd.Parse(argc, argv);

  Ptr<LteHelper> lteHelper = CreateObject<LteHelper> ();
  Ptr<PointToPointEpcHelper>  epcHelper = CreateObject<PointToPointEpcHelper> ();
  lteHelper->SetEpcHelper (epcHelper);

  ConfigStore inputConfig;
  inputConfig.ConfigureDefaults();
  cmd.Parse(argc, argv);

  Ptr<Node> pgw = epcHelper->GetPgwNode ();

  NodeContainer remoteHostContainer;
//  my server
  remoteHostContainer.Create (1);
  Ptr<Node> remoteHost = remoteHostContainer.Get (0);
  InternetStackHelper internet;
  internet.Install (remoteHostContainer);

  PointToPointHelper p2ph;
  p2ph.SetDeviceAttribute ("DataRate", DataRateValue (DataRate ("100Gb/s")));
  p2ph.SetDeviceAttribute ("Mtu", UintegerValue (1500));
  p2ph.SetChannelAttribute ("Delay", TimeValue (Seconds (0.010)));
  NetDeviceContainer internetDevices = p2ph.Install (pgw, remoteHost);
  Ipv4AddressHelper ipv4h;
  ipv4h.SetBase ("1.0.0.0", "255.0.0.0");
  Ipv4InterfaceContainer internetIpIfaces = ipv4h.Assign (internetDevices);
  Ipv4Address remoteHostAddr = internetIpIfaces.GetAddress (1);

  Ipv4StaticRoutingHelper ipv4RoutingHelper;
  Ptr<Ipv4StaticRouting> remoteHostStaticRouting = ipv4RoutingHelper.GetStaticRouting (remoteHost->GetObject<Ipv4> ());
  remoteHostStaticRouting->AddNetworkRouteTo (Ipv4Address ("7.0.0.0"), Ipv4Mask ("255.0.0.0"), 1);

  NodeContainer ueNodes;
  NodeContainer enbNodes;
  enbNodes.Create(numberOfNodes);
  ueNodes.Create(numberOfNodes);
  ueNodes.Get(0) = gateways.Get(0);
  
  Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();
  for (uint16_t i = 0; i < numberOfNodes; i++)
  {
    positionAlloc->Add (Vector(distance * i, 0, 0));
  }
  MobilityHelper mobility2;
  mobility2.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  mobility2.SetPositionAllocator(positionAlloc);
  mobility2.Install(enbNodes);
  mobility2.Install(ueNodes);

  NetDeviceContainer enbLteDevs = lteHelper->InstallEnbDevice (enbNodes);
  NetDeviceContainer ueLteDevs = lteHelper->InstallUeDevice (ueNodes);

  internet.Install (ueNodes);
  Ipv4InterfaceContainer ueIpIface;
  ueIpIface = epcHelper->AssignUeIpv4Address (NetDeviceContainer (ueLteDevs));
  for (uint32_t u = 0; u < ueNodes.GetN (); ++u)
  {
    Ptr<Node> ueNode = ueNodes.Get (u);
    Ptr<Ipv4StaticRouting> ueStaticRouting = ipv4RoutingHelper.GetStaticRouting (ueNode->GetObject<Ipv4> ());
    ueStaticRouting->SetDefaultRoute (epcHelper->GetUeDefaultGatewayAddress (), 1);
  }

  for (uint16_t i = 0; i < numberOfNodes; i++)
  {
    lteHelper->Attach (ueLteDevs.Get(i), enbLteDevs.Get(i));
  }

  uint16_t dlPort = 1234;
  uint16_t ulPort = 2000;
  uint16_t otherPort = 3000;
  ApplicationContainer clientApps;
  ApplicationContainer serverApps;
  for (uint32_t u = 0; u < ueNodes.GetN (); ++u)
  {
    ++ulPort;
    ++otherPort;
    PacketSinkHelper dlPacketSinkHelper ("ns3::UdpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), dlPort));
    PacketSinkHelper ulPacketSinkHelper ("ns3::UdpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), ulPort));
    PacketSinkHelper packetSinkHelper ("ns3::UdpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), otherPort));
    serverApps.Add (dlPacketSinkHelper.Install (ueNodes.Get(u)));
    serverApps.Add (ulPacketSinkHelper.Install (remoteHost));
    serverApps.Add (packetSinkHelper.Install (ueNodes.Get(u)));

    UdpClientHelper dlClient (ueIpIface.GetAddress (u), dlPort);
    dlClient.SetAttribute ("Interval", TimeValue (MilliSeconds(interPacketInterval)));
    dlClient.SetAttribute ("MaxPackets", UintegerValue(1000000));

    UdpClientHelper ulClient (remoteHostAddr, ulPort);
    ulClient.SetAttribute ("Interval", TimeValue (MilliSeconds(interPacketInterval)));
    ulClient.SetAttribute ("MaxPackets", UintegerValue(1000000));

    UdpClientHelper client (ueIpIface.GetAddress (u), otherPort);
    client.SetAttribute ("Interval", TimeValue (MilliSeconds(interPacketInterval)));
    client.SetAttribute ("MaxPackets", UintegerValue(1000000));

    clientApps.Add (dlClient.Install (remoteHost));
    clientApps.Add (ulClient.Install (ueNodes.Get(u)));
    if (u+1 < ueNodes.GetN ())
    {
      clientApps.Add (client.Install (ueNodes.Get(u+1)));
    }
    else
    {
      clientApps.Add (client.Install (ueNodes.Get(0)));
    }
  }
  serverApps.Start (Seconds (0.01));
  clientApps.Start (Seconds (0.01));


	
    
    std::vector<int> sfQuantity(6);
    sfQuantity = LorawanMacHelper::SetSpreadingFactorsUp(endDevices, gateways, channel);

    AnimationInterface anim ("animation_file.xml");

    Simulator::Stop(Hours(2));

    Simulator::Run();

    Simulator::Destroy();

    return 0;
}


