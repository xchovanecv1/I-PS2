/*
 Vytvorte casovu udalost (vramci prvych 30s), ktora rozpohybuje uzly v strede. (1b)
 Vytvorte funkciu ktorou zachytite zmeny RTO na triede TcpSocket (iba 1z3 app pouziva Tcp). (1b)
	Ak poklesne RTO zvyste rychlosti uzlov v strede. inak spomalte ;) (1b)
 Zachytte udalost (MacRxDrop || MacTxDrop) triedy ns3::WifiMac funkciou, kde (1b)
  vykreslite graf(<ktory uzol dropol>,t) ; t je cas simulacie (2b)
 
*/
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/mobility-module.h"
#include "ns3/config-store-module.h"
#include "ns3/wifi-module.h"
#include "ns3/internet-module.h"
#include "ns3/olsr-module.h"
#include "ns3/flow-monitor-module.h"
#include "myapp.h"

#include <iostream>
#include <fstream>
#include <vector>
#include <string>

using namespace ns3;



int main (int argc, char *argv[]){
  double distance = 500;  // m
  uint32_t numNodes = 25;  // by default, 5x5
  double interval = 0.001; // seconds
  uint32_t packetSize = 6000; // bytes
  uint32_t numPackets = 1000000000;
  std::string rtslimit = "1500";
  CommandLine cmd;

  cmd.AddValue ("distance", "distance (m)", distance);
  cmd.AddValue ("packetSize", "Packet size (B)", packetSize);
  cmd.AddValue ("rtslimit", "RTS/CTS Threshold (bytes)", rtslimit);
  cmd.Parse (argc, argv);

  Time interPacketInterval = Seconds (interval);

  // turn off RTS/CTS for frames below 2200 bytes
  Config::SetDefault ("ns3::WifiRemoteStationManager::RtsCtsThreshold", StringValue (rtslimit));
  Config::SetDefault ("ns3::WifiRemoteStationManager::FragmentationThreshold", StringValue ("2200"));
  //Config::SetDefault ("ns3::WifiRemoteStationManager::NonUnicastMode", StringValue (phyMode));

  NodeContainer c;
  c.Create (numNodes);

  WifiHelper wifi;

  YansWifiPhyHelper wifiPhy;
  wifiPhy.Set ("RxGain", DoubleValue (-10) );
  wifiPhy.SetErrorRateModel("ns3::YansErrorRateModel");
  YansWifiChannelHelper wifiChannel;
  wifiChannel.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");
  wifiChannel.AddPropagationLoss ("ns3::FriisPropagationLossModel");
  wifiPhy.SetChannel (wifiChannel.Create ());
  
  WifiMacHelper wifiMac;

  wifi.SetStandard (WIFI_PHY_STANDARD_80211b);
  wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager",
                                "DataMode",StringValue ("DsssRate1Mbps"),
                                "ControlMode",StringValue ("DsssRate1Mbps"));
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
  mobility.SetMobilityModel ("ns3::RandomWalk2dMobilityModel"
        ,"Mode", StringValue("Time")
        ,"Speed",StringValue( "ns3::ConstantRandomVariable[Constant=0.0]" )
        ,"Bounds", RectangleValue(Rectangle(0,distance*(numNodes/5),0,distance*(numNodes/5)))
  );
  mobility.Install (c);

  OlsrHelper routing;

  Ipv4ListRoutingHelper list;
  list.Add (routing, 10);

  InternetStackHelper internet;
  internet.SetRoutingHelper (list);
  internet.Install (c);

  Ipv4AddressHelper ipv4;
  ipv4.SetBase ("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer ifcont = ipv4.Assign (devices);



  uint16_t sinkPort = 6;

   Address sinkAddress1 (InetSocketAddress (ifcont.GetAddress (numNodes-1), sinkPort));
   PacketSinkHelper packetSinkHelper1 ("ns3::TcpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), sinkPort));
   ApplicationContainer sinkApps1 = packetSinkHelper1.Install (c.Get (numNodes-1));
   sinkApps1.Start (Seconds (0.));
   sinkApps1.Stop (Seconds (100.));

   Ptr<Socket> ns3UdpSocket1 = Socket::CreateSocket (c.Get (0), TcpSocketFactory::GetTypeId ());
   Ptr<MyApp> app1 = CreateObject<MyApp> ();
   app1->Setup (ns3UdpSocket1, sinkAddress1, packetSize, numPackets, DataRate ("1Mbps"));
   c.Get (0)->AddApplication (app1);
   app1->SetStartTime (Seconds (31.));
   app1->SetStopTime (Seconds (100.));

    Address sinkAddress2 (InetSocketAddress (ifcont.GetAddress (numNodes/5*3-1), sinkPort));
    PacketSinkHelper packetSinkHelper2 ("ns3::UdpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), sinkPort));
    ApplicationContainer sinkApps2 = packetSinkHelper2.Install (c.Get (numNodes/5*3-1));
    sinkApps2.Start (Seconds (0.));
    sinkApps2.Stop (Seconds (100.));
    Ptr<Socket> ns3UdpSocket2 = Socket::CreateSocket (c.Get (numNodes/5*2), UdpSocketFactory::GetTypeId ());
    Ptr<MyApp> app2 = CreateObject<MyApp> ();
    app2->Setup (ns3UdpSocket2, sinkAddress2, packetSize, numPackets, DataRate ("1Mbps"));
    c.Get (numNodes/5*2)->AddApplication (app2);
    app2->SetStartTime (Seconds (31.5));
    app2->SetStopTime (Seconds (100.));

    Address sinkAddress3 (InetSocketAddress (ifcont.GetAddress (numNodes/5*2-1), sinkPort));
    PacketSinkHelper packetSinkHelper3 ("ns3::UdpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), sinkPort));
    ApplicationContainer sinkApps3 = packetSinkHelper3.Install (c.Get (numNodes/5*2-1));
    sinkApps3.Start (Seconds (0.));
    sinkApps3.Stop (Seconds (100.));
    Ptr<Socket> ns3UdpSocket3 = Socket::CreateSocket (c.Get (numNodes/5*4), UdpSocketFactory::GetTypeId ());
    Ptr<MyApp> app3 = CreateObject<MyApp> ();
    app3->Setup (ns3UdpSocket3, sinkAddress3, packetSize, numPackets, DataRate ("1Mbps"));
    c.Get (numNodes/5*4)->AddApplication (app3);
    app3->SetStartTime (Seconds (32.));
    app3->SetStopTime (Seconds (100.));

  FlowMonitorHelper flowmon;
  Ptr<FlowMonitor> monitor = flowmon.InstallAll();
  
  

  Simulator::Stop (Seconds (100.0));
  Simulator::Run ();

  monitor->CheckForLostPackets ();
  Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier> (flowmon.GetClassifier ());
  std::map<FlowId, FlowMonitor::FlowStats> stats = monitor->GetFlowStats ();

  for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator iter = stats.begin (); iter != stats.end (); ++iter)
    {
	  Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow (iter->first);

/*
numNodes/5*4 -> numNodes/5*2-1  ( 21 -> 10)
numNodes/5*2 -> numNodes/5*3-1  ( 11 -> 15)
numNodes-1   -> 0               (  1 -> 25)
*/
      if ((t.sourceAddress == Ipv4Address("10.1.1.1") && t.destinationAddress == Ipv4Address("10.1.1.25"))
    	|| (t.sourceAddress == Ipv4Address("10.1.1.11") && t.destinationAddress == Ipv4Address("10.1.1.15"))
    	|| (t.sourceAddress == Ipv4Address("10.1.1.21") && t.destinationAddress == Ipv4Address("10.1.1.10")))
        {
    	  std::cout << "Flow ID: " << iter->first << " Src Addr " << t.sourceAddress << " Dst Addr " << t.destinationAddress << "\n";
    	  std::cout << "Tx Packets = " << iter->second.txPackets << "\n";
    	  std::cout << "Rx Packets = " << iter->second.rxPackets << "\n";
    	  std::cout << "Throughput: " << iter->second.rxBytes * 8.0 / (iter->second.timeLastRxPacket.GetSeconds()-iter->second.timeFirstTxPacket.GetSeconds()) / 1024  << " Kbps\n";
        }
    }

   
  Simulator::Destroy ();

  return 0;
}

