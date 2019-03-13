/*
Ulohy
- Doplnte kod aby vystupom bol subor vizualnej simulacie Netanim. Uzol zdroja nazvyte 'Z' a uzol ciela 'K'. (1b)
- - Doplnte moznost zmeny parametra (bez nutnosti zostavovania projektu) vysky anteny pre model TwoRayGroundPropagationLossModel. (1b)
- Odsimulujte a vykreslite do grafu (1b) zavislost priepustnosti (Throughput) od velkosti posielaneho balika aplikacnej vrstve (PacketSize) (1b)
 */


#include "ns3/config.h"
#include "ns3/uinteger.h"
#include "ns3/double.h"
#include "ns3/string.h"
#include "ns3/yans-wifi-helper.h"
#include "ns3/mobility-helper.h"
#include "ns3/ipv4-address-helper.h"
#include "ns3/yans-wifi-channel.h"
#include "ns3/mobility-model.h"
#include "ns3/olsr-helper.h"
#include "ns3/ipv4-static-routing-helper.h"
#include "ns3/ipv4-list-routing-helper.h"
#include "ns3/internet-stack-helper.h"
#include "ns3/on-off-helper.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/log.h"


using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("MenoPriezvisko");

int main (int argc, char *argv[])
{
  std::string phyMode ("DsssRate1Mbps");
  double distance = 500;  // m
  uint32_t packetSize = 1000; // bytes
  uint32_t numPackets = 1;
  uint32_t numNodes = 25;  // by default, 5x5
  uint32_t sinkNode = 0;
  uint32_t sourceNode = 24;
  double interval = 1.0; // seconds
 
   // Convert to time object
  Time interPacketInterval = Seconds (interval);

  // Fix non-unicast data rate to be the same as that of unicast
  Config::SetDefault ("ns3::WifiRemoteStationManager::NonUnicastMode",
                      StringValue (phyMode));

  NodeContainer c;
  c.Create (numNodes);
  
  // The below set of helpers will help us to put together the wifi NICs we want
  WifiHelper wifi;
  
  YansWifiPhyHelper wifiPhy =  YansWifiPhyHelper::Default ();
  // set it to zero; otherwise, gain will be added
  wifiPhy.Set ("RxGain", DoubleValue (-10) );
  // ns-3 supports RadioTap and Prism tracing extensions for 802.11b
  wifiPhy.SetPcapDataLinkType (WifiPhyHelper::DLT_IEEE802_11_RADIO);

  YansWifiChannelHelper wifiChannel;
  wifiChannel.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");
  wifiChannel.AddPropagationLoss ("ns3::TwoRayGroundPropagationLossModel");
  wifiPhy.SetChannel (wifiChannel.Create ());

  // Add an upper mac and disable rate control
  WifiMacHelper wifiMac;
  wifi.SetStandard (WIFI_PHY_STANDARD_80211b);
  wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager",
                                "DataMode",StringValue (phyMode),
                                "ControlMode",StringValue (phyMode));
  // Set it to adhoc mode
  wifiMac.SetType ("ns3::AdhocWifiMac");
  NetDeviceContainer devices = wifi.Install (wifiPhy, wifiMac, c);

  MobilityHelper mobility;
  mobility.SetPositionAllocator ("ns3::GridPositionAllocator",
                                 "MinX", DoubleValue (0.0),
                                 "MinY", DoubleValue (0.0),
                                 "DeltaX", DoubleValue (distance),
                                 "DeltaY", DoubleValue (distance),
                                 "GridWidth", UintegerValue (5),
                                 "LayoutType", StringValue ("RowFirst"));
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (c);

  // Enable OLSR
  OlsrHelper olsr;
  Ipv4StaticRoutingHelper staticRouting;

  Ipv4ListRoutingHelper list;
  list.Add (staticRouting, 0);
  list.Add (olsr, 10);

  InternetStackHelper internet;
  internet.SetRoutingHelper (list); // has effect on the next Install ()
  internet.Install (c);

  Ipv4AddressHelper ipv4;
  NS_LOG_INFO ("Assign IP Addresses.");
  ipv4.SetBase ("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer i = ipv4.Assign (devices);

  TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
  Ptr<Socket> recvSink = Socket::CreateSocket (c.Get (sinkNode), tid);
  InetSocketAddress local = InetSocketAddress (Ipv4Address::GetAny (), 80);
  recvSink->Bind (local);
  //recvSink->SetRecvCallback (MakeCallback (&ReceivePacket));

  Ptr<Socket> source = Socket::CreateSocket (c.Get (sourceNode), tid);
  InetSocketAddress remote = InetSocketAddress (i.GetAddress (sinkNode, 0), 80);
  source->Connect (remote);

  // Output what we are doing
  NS_LOG_UNCOND ("Testing from node " << sourceNode << " to " << sinkNode << " with grid distance " << distance);
  
   ApplicationContainer cbrApps;
   uint16_t cbrPort = 9;
   OnOffHelper onOffHelper("ns3::TcpSocketFactory", InetSocketAddress (
           i.GetAddress(sinkNode)
           , cbrPort));
   onOffHelper.SetAttribute ("PacketSize", UintegerValue (packetSize));
   onOffHelper.SetAttribute ("OnTime",  StringValue ("ns3::ConstantRandomVariable[Constant=1]"));
   onOffHelper.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));
   onOffHelper.SetAttribute ("DataRate", StringValue ("1000000bps"));
   onOffHelper.SetAttribute ("StartTime", TimeValue (Seconds (30.0)));
   cbrApps.Add (onOffHelper.Install (c.Get (sourceNode)));

   
  FlowMonitorHelper flowmon;
   Ptr<FlowMonitor> monitor = flowmon.Install(c);
  flowmon.SerializeToXmlFile("a.flowmon", true, true);
  
  Simulator::Stop (Seconds (33.0));
  Simulator::Run ();
  
   monitor->CheckForLostPackets ();
   Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier> (flowmon.GetClassifier ());
   FlowMonitor::FlowStatsContainer stats = monitor->GetFlowStats ();
   
   for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator i = stats.begin (); i != stats.end (); ++i)
     {     Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow (i->first);
           std::cout << "  Throughput: " << i->second.rxBytes * 8.0 / 3.0 / 1000 << " kbps\n";
    }
  
  
  Simulator::Destroy ();

  return 0;
}

