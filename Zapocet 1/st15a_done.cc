/*
Ulohy
- Doplnte kod aby vystupom bol subor vizualnej simulacie Netanim. Uzol zdroja nazvyte 'Z' a uzol ciela 'K'. (1b)
- - Doplnte moznost zmeny parametra (bez nutnosti zostavovania projektu) systemovych strat signalu v mediu pre model FriisPropagationLossModel. (1b)
- Odsimulujte a vykreslite do grafu (1b) zavislost priepustnosti (Throughput) od poctu uzlov (numNodes) (1b)
(pre krajsiu simulaciu)
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
#include "ns3/netanim-module.h"
#include "ns3/core-module.h"
#include "ns3/gnuplot.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("ViktorChovanec");

void experiment(uint32_t pocet_u, Gnuplot2dDataset& data, Gnuplot2dDataset& data2, double sysLos, bool rtscts) {
    std::string phyMode ("DsssRate1Mbps");
  double distance = 500;  // m
  uint32_t packetSize = 1000; // bytes
  uint32_t numPackets = 1;
  uint32_t numNodes = pocet_u;  // by default, 5x5 // 25
  uint32_t sinkNode = 0;
  uint32_t sourceNode = pocet_u-1;

  double interval = 1.0; // seconds

   // Convert to time object
  Time interPacketInterval = Seconds (interval);
  
  /*if(rtscts)
    Config::SetDefault ("ns3::WifiRemoteStationManager::FragmentationThreshold", UintegerValue (100));
  */
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
  wifiChannel.AddPropagationLoss ("ns3::FriisPropagationLossModel", "SystemLoss", DoubleValue(sysLos));
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
/*
  MobilityHelper mobility;
  Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();
  for(uint32_t i,j,x= 0; x< numNodes; x++){
	i=x/5; j=x%5;
	positionAlloc->Add (Vector (i*distance, j*distance, 0.0));
  }*/

  MobilityHelper mobility;
  Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();
  for(uint32_t i,j,x= 0; x< numNodes; x++){
	positionAlloc->Add (Vector (i*distance, 0, 0.0));
  }
  
  mobility.SetPositionAllocator (positionAlloc);
  
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
  
  AnimationInterface anim ("st15-anim.xml");
  
  for(uint32_t i= 0; i< numNodes; i++){
        anim.SetConstantPosition(c.Get (i), i*distance, 0/*,0*/);
  }
  anim.UpdateNodeDescription (c.Get (sinkNode), "K");
  anim.UpdateNodeDescription (c.Get (sourceNode), "Z");
  /*anim.UpdateNodeName
  uint32_t sinkNode = 0;
  uint32_t sourceNode = 24;*/
   /*
    Ptr<Node> apWifiNode = networkNodes.Get (0);
    Ptr<Node> staWifiNode = networkNodes.Get (1);
    */
   //anim.UpdateNodeColor (c.Get(), 255, 0, 0);
   //anim.UpdateNodeColor (staWifiNode, 0, 255, 0);
  
  Simulator::Stop (Seconds (33.0));
  Simulator::Run ();
  
   monitor->CheckForLostPackets ();
   Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier> (flowmon.GetClassifier ());
   FlowMonitor::FlowStatsContainer stats = monitor->GetFlowStats ();
   
   int poc=0;
   for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator i = stats.begin (); i != stats.end (); ++i)
     {     Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow (i->first);
     double troughput = i->second.rxBytes * 8.0 / 3.0 / 1000;
     if(poc == 0)
        data.Add(pocet_u,troughput);
     else 
        data2.Add(pocet_u,troughput);
     poc++;
           std::cout << "  Throughput: " << troughput << " kbps\n";
    }
  
  
  Simulator::Destroy ();
}

int main (int argc, char *argv[])
{
    double sysLoss = 1;

    CommandLine cmd;
    cmd.AddValue ("sysLoss", "The system loss", sysLoss);

    cmd.Parse (argc,argv); 
    
  Gnuplot graf("graf.svg");
    graf.SetTerminal("svg");
    graf.SetTitle("Zavislost priepustnosti od poctu uzlov");
    graf.SetLegend("Pocet uzlov [ks]","Priepustnost [kbit/s]");
    graf.AppendExtra("set xrange[10:25]");
    graf.AppendExtra("set yrange[0.1:0.2]");
    Gnuplot2dDataset data;
    data.SetTitle ("Zavislost priepustnosti od poctu uzlov");
    data.SetStyle (Gnuplot2dDataset::LINES_POINTS);

    Gnuplot2dDataset data2;
    data.SetTitle ("Zavislost priepustnosti");
    data.SetStyle (Gnuplot2dDataset::LINES_POINTS);

    for(int i= 10; i < 25; i += 1)
        experiment(i, data, data2, sysLoss, true);

    graf.AddDataset (data);
    graf.AddDataset (data2);
    std::ofstream plotFile ("graf.plt");
    graf.GenerateOutput (plotFile);
    plotFile.close ();
    if(system("gnuplot graf.plt"));
  return 0;
}

