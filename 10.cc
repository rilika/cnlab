#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/ipv4-global-routing-helper.h"

                       

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("SecondScriptExample");

int main (int argc, char *argv[]){
    bool verbose = true;
    uint32_t nCsma = 2;
    CommandLine cmd;
    cmd.AddValue ("nCsma", "Number of \"extra\" CSMA nodes/devices", nCsma);
    cmd.AddValue ("verbose", "Tell echo applications to log if true", verbose);
    cmd.Parse (argc,argv);

    if (verbose){
        LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
        LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);
    }
    nCsma = nCsma == 0? 1 : nCsma;

    NodeContainer p2pNodes, p2pNodes1, csmaNodes;
    p2pNodes.Create (3); 
   
    csmaNodes.Add (p2pNodes.Get (2)); // added n1
    csmaNodes.Create (nCsma); // created n2 and n3
   
    PointToPointHelper pointToPoint;
    pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
    pointToPoint.SetChannelAttribute ("Delay", StringValue ("2ms"));
    CsmaHelper csma;
    csma.SetChannelAttribute ("DataRate", StringValue ("100Mbps"));
    csma.SetChannelAttribute ("Delay", TimeValue (NanoSeconds (6560)));
    PointToPointHelper pointToPoint1;
    pointToPoint1.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
    pointToPoint1.SetChannelAttribute ("Delay", StringValue ("2ms"));
   
    NetDeviceContainer p2pDevices, p2pDevices1, csmaDevices;// p2pDevices2, p2pDevices3;
    p2pDevices = pointToPoint.Install (p2pNodes.Get(0), p2pNodes.Get(1));
    p2pDevices1 = pointToPoint1.Install (p2pNodes.Get(1), p2pNodes.Get(2));
    csmaDevices = csma.Install (csmaNodes);
   
    InternetStackHelper stack;
    stack.Install (p2pNodes.Get (0)); // node m0 and m1
    stack.Install (p2pNodes.Get (1)); // node m1
    stack.Install (csmaNodes); // nodes n1,n2, n3, n4
   Ipv4AddressHelper address;
    Ipv4InterfaceContainer p2pInterfaces, p2pInterfaces1, csmaInterfaces;// p2pInterfaces2, p2pInterfaces3;
    address.SetBase ("20.1.1.0", "255.255.255.0");
    p2pInterfaces = address.Assign (p2pDevices);
    p2pInterfaces1 = address.Assign (p2pDevices1);
    address.SetBase ("30.1.2.0", "255.255.255.0");
    csmaInterfaces = address.Assign (csmaDevices);
 
    UdpEchoServerHelper echoServer (9);// echoServer1(10); // 9 and 10 are port numbers

    ApplicationContainer serverApps = echoServer.Install (csmaNodes.Get (nCsma));
    serverApps.Start (Seconds (1.0));
    serverApps.Stop (Seconds (10.0));
 
    UdpEchoClientHelper echoClient (csmaInterfaces.GetAddress (nCsma), 9);
    echoClient.SetAttribute ("MaxPackets", UintegerValue (1));
    echoClient.SetAttribute ("Interval", TimeValue (Seconds (1.0)));
    echoClient.SetAttribute ("PacketSize", UintegerValue (1024));
    ApplicationContainer clientApps = echoClient.Install (p2pNodes.Get (1));
    clientApps.Start (Seconds (2.0));
    clientApps.Stop (Seconds (10.0));
 
    Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

    pointToPoint.EnablePcapAll ("second");
    csma.EnablePcap ("second", csmaDevices.Get (1), true);
    pointToPoint1.EnablePcapAll ("second");
   
    Simulator::Run ();
    Simulator::Destroy ();
    return 0;
}
