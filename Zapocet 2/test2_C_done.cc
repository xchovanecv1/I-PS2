/*
- Pokyny: Stiahninete si nasledovny subor.
- Vytvorte casovu udalost, ktora bude zachytena funkciou s vlastnym argumentom Ptr<Application>. ( 1b, 1b)
- Zachytte udalost zmeny okna zahltenia (CongestionWindow), 
pomocka: CongestionWindow je nazov udalosti v triede TcpSocket, tu mozno zavolat az ked je socket vytvoreny t.j. v case simulacie. Bud priamo cez objekt typu Socket, kt. mozno ziskat z aplikacie.
app->GetObject<OnOffAplication>()->GetSocket(); na ktorom existuje clenska funkcia umoznenie zachytavania udalosti.
(1b)
   vykreslite graf zavislosti velkosti okna zahltenia v case. (1b)
-- Ak nastane pokles velkosti zahltenia, zmente na vsetkych uzloch interval Hello sprav, aby sa neposielali tak casto,
    a tym uvolnili kanal pre aplikaciu. (2b)
*/
 
#include "ns3/core-module.h"
#include "ns3/mobility-module.h"
#include "ns3/wifi-module.h"
#include "ns3/network-module.h"
#include "ns3/aodv-module.h"
#include "ns3/internet-module.h"
#include "ns3/applications-module.h"
#include "ns3/gnuplot.h"

using namespace ns3;
using namespace std;

Gnuplot2dDataset data;

void CwndChange(uint32_t oldValue,uint32_t newValue ){
    //cout << newValue << endl;
    data.Add(Simulator::Now().GetSeconds(), newValue);
    
    if(newValue < oldValue){
        Config::Set("/NodeList/*/$ns3::aodv::RoutingProtocol/HelloInterval", TimeValue(Seconds(0.001)));
    }
}

static void CasovaUdalost(Ptr<Application> app){
    cout << "zachytena casova udalost" << endl;
    app->GetObject<OnOffApplication>()->GetSocket()->TraceConnectWithoutContext("CongestionWindow", MakeCallback(&CwndChange));
}
 

int main (int argc, char *argv[]){
  double simulationTime = 10; //seconds
  double distance = 5; //meters
  bool enableRts = 0;
   
  CommandLine cmd;
  cmd.AddValue ("enableRts", "Enable or disable RTS/CTS", enableRts);
  cmd.AddValue ("simulationTime", "Simulation time in seconds", simulationTime);
  cmd.AddValue ("distance", "Distance in meters between the station and the access point", distance);
  cmd.Parse (argc, argv);
 
  Config::SetDefault ("ns3::WifiRemoteStationManager::RtsCtsThreshold"
    , enableRts ? StringValue ("0") : StringValue ("999999"));
 
  NodeContainer wifiStaNodes;
  wifiStaNodes.Create (2);
  NodeContainer wifiApNodes;
  wifiApNodes.Create (1);
 
  YansWifiChannelHelper channel = YansWifiChannelHelper::Default ();
  YansWifiPhyHelper phy = YansWifiPhyHelper::Default ();
  phy.SetPcapDataLinkType (WifiPhyHelper::DLT_IEEE802_11_RADIO);
  phy.SetChannel (channel.Create ());
 
  WifiHelper wifi;
  wifi.SetStandard (WIFI_PHY_STANDARD_80211n_5GHZ);
  wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager"
            , "DataMode", StringValue ("HtMcs7")
            , "ControlMode", StringValue ("HtMcs0")
  );
  WifiMacHelper mac;
 
  NetDeviceContainer staDeviceA, apDeviceA;
 
  phy.Set ("ChannelNumber", UintegerValue (36));
  mac.SetType ("ns3::AdhocWifiMac");
  staDeviceA = wifi.Install (phy, mac, wifiStaNodes.Get (0));
  apDeviceA = wifi.Install (phy, mac, wifiApNodes.Get (0));
 
  MobilityHelper mobility;
  Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
 
  positionAlloc->Add (Vector (0.0, 0.0, 0.0));
  positionAlloc->Add (Vector (distance, 0.0, 0.0));
  positionAlloc->Add (Vector (10 + distance, 0.0, 0.0));
 
  mobility.SetPositionAllocator (positionAlloc);
  mobility.Install (wifiApNodes);
  mobility.SetMobilityModel ("ns3::RandomWalk2dMobilityModel",
            "Bounds", StringValue("-50|50|-50|50")//RectangleValue (Rectangle(-50, 50, -50, 50))
  );
  mobility.Install (wifiStaNodes);
   
  AodvHelper aodv;
  InternetStackHelper stack;
  stack.SetRoutingHelper (aodv);
  stack.Install (wifiApNodes);
  stack.Install (wifiStaNodes);
 
  Ipv4AddressHelper address;
  address.SetBase ("192.168.1.0", "255.255.255.0");
  Ipv4InterfaceContainer StaInterfaceA;
  StaInterfaceA = address.Assign (staDeviceA);
  Ipv4InterfaceContainer ApInterfaceA;
  ApInterfaceA = address.Assign (apDeviceA);
 
  uint16_t port = 9;
  Address sinkAddress (InetSocketAddress (ApInterfaceA.GetAddress (0),port));
  PacketSinkHelper packetSinkHelper ("ns3::TcpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), port));
  ApplicationContainer sinkApps = packetSinkHelper.Install (wifiApNodes.Get (0));
  sinkApps.Start (Seconds (0.));
  sinkApps.Stop (Seconds (60.));
 
  OnOffHelper onOffHelper ("ns3::TcpSocketFactory", sinkAddress);
  onOffHelper.SetAttribute ("OnTime", StringValue("ns3::ConstantRandomVariable[Constant=1]"));
  onOffHelper.SetAttribute ("OffTime", StringValue("ns3::ConstantRandomVariable[Constant=0]"));
  onOffHelper.SetAttribute ("DataRate",StringValue ("54Mbps"));
  onOffHelper.SetAttribute ("PacketSize", UintegerValue (1024));
   
  ApplicationContainer clientAppA; 
  clientAppA.Add(onOffHelper.Install (wifiStaNodes.Get (0)));
  clientAppA.Start (Seconds (1.0));
  clientAppA.Stop (Seconds (simulationTime + 1));
 
  //casova udalost
  Simulator::Schedule (Seconds (1.0 + 0.1), &CasovaUdalost, clientAppA.Get(0));
 
  Simulator::Stop (Seconds (simulationTime + 1));
  Simulator::Run ();
 
  Simulator::Destroy ();
  
  
  Gnuplot graf("graf.svg");
  graf.SetTerminal("svg");	
  graf.SetLegend("Cas","Zahltenie okna");
  graf.SetTitle("Zavislost zahltenia okna od casu");

  graf.AddDataset (data);
  std::ofstream plotFile ("graf.gnuplot");
  graf.GenerateOutput (plotFile);
  plotFile.close ();
  if(system("gnuplot graf.gnuplot"));  
 
  return 0;
}