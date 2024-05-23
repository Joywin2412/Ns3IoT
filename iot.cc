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
    allocator->Add(Vector(0, 0, 0));

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

    // Install mobility model for the gateway
    mobility.Install(gateways);

    // Create a netdevice for the gateway
    phyHelper.SetDeviceType(LoraPhyHelper::GW);
    macHelper.SetDeviceType(LorawanMacHelper::GW);
    helper.Install(phyHelper, macHelper, gateways);

    Ptr<LteHelper> lteHelper = CreateObject<LteHelper>();


   NodeContainer enbNodes;
   enbNodes.Create(1);
   NodeContainer ueNodes;
   ueNodes.Create(2);

   uint16_t port = 9;
   UdpServerHelper server(port);
   ApplicationContainer serverApp = server.Install(ueNodes.Get(1));
   serverApp.Start(Seconds(1.0));
    serverApp.Stop(Seconds(20.0));


    ueNodes.Get(0) = gateways.Get(0);
    NetDeviceContainer enbDevs;
    enbDevs = lteHelper->InstallEnbDevice(enbNodes);

    NetDeviceContainer ueDevs;
    ueDevs = lteHelper->InstallUeDevice(ueNodes);

    lteHelper->Attach(ueDevs, enbDevs.Get(0));
lteHelper->Attach(ueDevs, enbDevs.Get(1));

    enum EpsBearer::Qci q = EpsBearer::GBR_CONV_VOICE;
    EpsBearer bearer(q);
    lteHelper->ActivateDataRadioBearer(ueDevs, bearer);

	
    /******************
     * Set Data Rates *
     ******************/
    std::vector<int> sfQuantity(6);
    sfQuantity = LorawanMacHelper::SetSpreadingFactorsUp(endDevices, gateways, channel);

    /****************
     *  Simulation  *
     ****************/

    Simulator::Stop(Hours(2));

    Simulator::Run();

    Simulator::Destroy();

    return 0;
}

