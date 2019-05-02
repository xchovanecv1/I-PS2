/*
3. Simulácia elektronickej hliadky.
Robot (UAV alebo ine) sa pohybuje po sklade a vysiela signál videa (streaming) na server. V sklade sa nachadzajú AP s priamim prístupom na server.

Abstrakcia: 
- Majme sklad tvaru obdlžnik. 
- AP sú rovnomerne rozložené po sklade. 
- Dron sa náhodne pohybuje po sklade vysiela údaje na server. + Ak dron nedokáže vysielať signál vráti sa "domov"
-východiskový bod pokrytý AP Na vizualizáciu simulácie využite program a modul NetAnim. Zadefinujte aspoň 2 merané parametre QoS, ktoré vynesiete do grafov za pomoci modulu a programu Gnuplot. Simuláciu spustite viac krát, aby ste ziskali aj štandardnú odchylku merania pre vynesené body grafu. Vhodne stanovte čas
-trvanie simulácie, rozsahy parametrov (fyzických, protokolov ...).

Príklad QoS (pomer priemerneho poctu poslaných uzitocných údajov k celkovému poctu poslaných údajov na jednu cestu/balik), priklad parametra pocet vedlajsich komunikacii v sieti.
*/


#include "ns3/core-module.h"
#include "ns3/mobility-module.h"
#include "ns3/wifi-module.h"
#include "ns3/internet-module.h"
#include "ns3/olsr-helper.h"
#include "ns3/olsr-routing-protocol.h"
#include "ns3/ipv4-routing-protocol.h"
#include "ns3/olsr-helper.h"
#include "ns3/netanim-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include <string>

using namespace ns3;
using namespace std;

NS_LOG_COMPONENT_DEFINE ("WifiSimpleAdhocGrid");


NodeContainer node_cont;
Ipv4InterfaceContainer ip_container

static void GenerateTraffic (Ptr<Socket> socket, uint32_t pktSize, 
                             uint32_t pktCount, Time pktInterval )
{

  if (pktCount > 0)
    {
      uint8_t pici[] = "chlpatvy";
      socket->Send (Create<Packet> (pici, 8));
      Simulator::Schedule (pktInterval, &GenerateTraffic, 
                           socket, pktSize,pktCount-1, pktInterval);
    }
  else
    {
      socket->Close ();
    }
}


void ReceivePacket (Ptr<Socket> socket)
{
    Ptr<Packet> rcv;
 
    Ptr<Node> trt = socket->GetNode();
    cout << trt->GetId() << endl;
  TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
  while (( rcv = socket->Recv ()))
    {
      
      uint32_t size = rcv->GetSize();
      uint8_t * data = (uint8_t*)malloc(size);
      /*cout << rcv->ToString() << " " << rcv->GetSize()  << endl;
      NS_LOG_UNCOND ("Received one packet!");
      cout << "Bravcova pata" << endl;*/
      rcv->CopyData (data, size);
      cout << "Magian: " << data << endl;
             /*
      Ptr<Socket> source = Socket::CreateSocket (trt, tid);
	  InetSocketAddress remote = InetSocketAddress (Ipv4Address ("255.255.255.255"), 80);
	  source->SetAllowBroadcast (true);
	  source->Connect (remote);
          GenerateTraffic (source, 20,1, Seconds(1));
          source->Close();*/
      free(data);
    }
}


static void GenerateTraffic (Ptr<Socket> socket)
{/*
  if (pktCount > 0)
    {*/
      uint8_t dta[] = "Za boha, za narod";
      
      socket->Send (Create<Packet> (dta, 18));
      //Simulator::Schedule (pktInterval, &GenerateTraffic, socket, pktSize,pktCount - 1, pktInterval);
 /*   }
  else
    {
      socket->Close ();
    }*/
}


static void GenerateDataN (Ptr<Socket> socket, int hop_c, uint8_t* data, uint8_t len)
{/*
  if (pktCount > 0)
    {*/
      uint8_t dta[] = "Za boha, za narod";
      uint8_t* packet = (uint8_t*)malloc(600);
      
      sprintf((char*)packet, "%04d%04d:%s",1,hop_c,data);
      socket->Send (Create<Packet> (packet, 600));
      //Simulator::Schedule (pktInterval, &GenerateTraffic, socket, pktSize,pktCount - 1, pktInterval);
 /*   }
  else
    {
      socket->Close ();
    }*/
}

static void SendDataToNeighbours(Ptr< Node > sNode) {
  
    Ptr<Ipv4> stack = sNode->GetObject<Ipv4> ();
  Ptr<Ipv4RoutingProtocol> rp_Gw = (stack->GetRoutingProtocol ());
  Ptr<Ipv4ListRouting> lrp_Gw = DynamicCast<Ipv4ListRouting> (rp_Gw);

  Ptr<olsr::RoutingProtocol> olsrrp_Gw;

  for (uint32_t i = 0; i < lrp_Gw->GetNRoutingProtocols ();  i++)
    {
      int16_t priority;
      Ptr<Ipv4RoutingProtocol> temp = lrp_Gw->GetRoutingProtocol (i, priority);
      if (DynamicCast<olsr::RoutingProtocol> (temp))
        {
          olsrrp_Gw = DynamicCast<olsr::RoutingProtocol> (temp);
        }
    }
  
  olsrrp_Gw->Dump();
  
  TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");

  
  std::vector< olsr::RoutingTableEntry > susedi = olsrrp_Gw-> GetRoutingTableEntries();
  for(int d = 0; d < susedi.size(); d++) {
      olsr::RoutingTableEntry bf = susedi.at(d);
      
      if(bf.distance == 1) {

        Ptr<Socket> source = Socket::CreateSocket (sNode, tid);
        // 
        InetSocketAddress remote = InetSocketAddress (bf.destAddr, 80);
        source->Connect (remote);

        //GenerateTraffic(source);
        uint8_t data[] = "koni trt";
        GenerateDataN(source, 1, data, 9);
        source->Close();
      }
      
      //cout << bf.destAddr << ": " << bf.distance << endl;
  }
  cout << susedi.size() << " pic" << endl;
  cout << "ends" << endl;
}



string WalkBounds (uint32_t minX, uint32_t maxX, uint32_t minY, uint32_t maxY)
{
  return std::to_string(minX) + "|" + std::to_string(maxX) + "|" + std::to_string(minY) + "|" + std::to_string(maxY);
}

void SendSeckym(Ptr<Node> sender, Ptr<Node> reciever, Ipv4Address rcvAddr) {
  
     Time interPacketInterval = Seconds (1.0);
  TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
  Ptr<Socket> source = Socket::CreateSocket (sender, tid);
  // 
  InetSocketAddress remote = InetSocketAddress (rcvAddr, 80);
  source->Connect (remote);
  GenerateTraffic(source, 50, 1, interPacketInterval);
  
}
  


int main (int argc, char *argv[])
{
  std::string phyMode ("DsssRate1Mbps");
  double distance = 500;  // m
  uint32_t packetSize = 1000; // bytes
  uint32_t numPackets = 1;
  uint32_t numNodes = 25;  // by default, 5x5
  uint32_t numNodesInRow = 5;
  uint32_t sinkNode = 0;
  uint32_t sourceNode = 1;
  double interval = 1.0; // seconds
  bool verbose = false;
  bool tracing = false;
  
  uint32_t sym_time = 43;
  uint32_t send_time = 30;
  uint32_t send_time_i = 5;

  uint32_t m_maxX = (distance) * numNodesInRow;
  uint32_t m_minX = 0;
  uint32_t m_maxY = (distance) * numNodesInRow;
  uint32_t m_minY = 0;
  
  //define vars for ap points
  MobilityHelper mobility;
  WifiHelper wifi;

  //define for drone
  WifiHelper wifi2;
  MobilityHelper mobility2;
  NodeContainer ap2;
  PacketSocketHelper packetSocket;

  
  packetSocket.Install (ap2);

  // Convert to time object
  Time interPacketInterval = Seconds (interval);

  // disable fragmentation for frames below 2200 bytes
  Config::SetDefault ("ns3::WifiRemoteStationManager::FragmentationThreshold", StringValue ("2200"));
  // turn off RTS/CTS for frames below 2200 bytes
  Config::SetDefault ("ns3::WifiRemoteStationManager::RtsCtsThreshold", StringValue ("2200"));
  // Fix non-unicast data rate to be the same as that of unicast
  Config::SetDefault ("ns3::WifiRemoteStationManager::NonUnicastMode",
                      StringValue (phyMode));

  node_cont.Create (numNodes);

  // The below set of helpers will help us to put together the wifi NICs we want
  
  if (verbose)
    {
      wifi.EnableLogComponents ();  // Turn on all Wifi logging
    }

  YansWifiPhyHelper wifiPhy =  YansWifiPhyHelper::Default ();
  // set it to zero; otherwise, gain will be added
  wifiPhy.Set ("RxGain", DoubleValue (-10) );
  // ns-3 supports RadioTap and Prism tracing extensions for 802.11b
  wifiPhy.SetPcapDataLinkType (YansWifiPhyHelper::DLT_IEEE802_11_RADIO);

  YansWifiChannelHelper wifiChannel;
  wifiChannel.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");
  wifiChannel.AddPropagationLoss ("ns3::FriisPropagationLossModel");
  wifiPhy.SetChannel (wifiChannel.Create ());

  // Add an upper mac and disable rate control
  WifiMacHelper wifiMac;
  wifi.SetStandard (WIFI_PHY_STANDARD_80211b);
  wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager",
                                "DataMode",StringValue (phyMode),
                                "ControlMode",StringValue (phyMode));
  // Set it to adhoc mode
  wifiMac.SetType ("ns3::AdhocWifiMac");
  NetDeviceContainer devices = wifi.Install (wifiPhy, wifiMac, node_cont);
/*
  GridPositionAllocator posAllocator;
  posAllocator.SetMinX(0.0);
  posAllocator.SetMinY(0.0);
  posAllocator.SetDeltaX(distance);
  posAllocator.SetDeltaY(distance);
  posAllocator.SetGridWidth
                                 "GridWidth", UintegerValue (5),
  posAllocator.SetLayoutType(ns3::GridPositionAllocator::ROW_FIRST);
  
  */
  mobility.SetPositionAllocator ("ns3::GridPositionAllocator",
                                 "MinX", DoubleValue (0.0),
                                 "MinY", DoubleValue (0.0),
                                 "DeltaX", DoubleValue (distance),
                                 "DeltaY", DoubleValue (distance),
                                 "GridWidth", UintegerValue (5),
                                 "LayoutType", StringValue ("RowFirst"));

  mobility.SetMobilityModel ("ns3::RandomWalk2dMobilityModel",
                            "Mode", StringValue ("Time"),
                            "Time", StringValue ("2s"),
                            "Direction", StringValue ("ns3::UniformRandomVariable[Min=0.0|Max=6.283184]"),
                            "Speed", StringValue ("ns3::UniformRandomVariable[Min=0.0|Max=0]"),
                            "Bounds", StringValue (WalkBounds(m_minX,m_maxX,m_minY,m_maxY)));
  mobility.Install (node_cont);

  // Enable OLSR
  OlsrHelper olsr;
  Ipv4StaticRoutingHelper staticRouting;

  Ipv4ListRoutingHelper list;
  list.Add (staticRouting, 0);
  list.Add (olsr, 10);

  InternetStackHelper internet;
  internet.SetRoutingHelper (list); // has effect on the next Install ()
  internet.Install (node_cont);

  Ipv4AddressHelper ipv4;
  NS_LOG_INFO ("Assign IP Addresses.");
  ipv4.SetBase ("10.1.1.0", "255.255.255.0");
  
  ip_container = ipv4.Assign (devices);


  TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");

  cout << "Jebko " <<Ipv4Address::GetAny () << endl;
  
  for(int n = 0; n < numNodes; n++) {
    Ptr<Socket> recvSink = Socket::CreateSocket (node_cont.Get (n), tid);
    InetSocketAddress local = InetSocketAddress (Ipv4Address::GetAny (), 80);
    recvSink->Bind (local);
    recvSink->SetAllowBroadcast(true);
    recvSink->SetRecvCallback (MakeCallback (&ReceivePacket));

  }
  /*
  for(uint32_t st = send_time; st < (sym_time-1); st+=send_time_i) {
      for(uint32_t ns = 0; ns < numNodes; ns++) {
          for(uint32_t nr = 0; nr < numNodes; nr++) {
              if(ns != nr) {
                Simulator::Schedule(Seconds (st+((nr+ns)*0.1)), &SendSeckym, c.Get(ns), c.Get(nr),i.GetAddress(nr,0));
              }
          }
      }
  }
  */
  /*
  Ptr<Socket> source = Socket::CreateSocket (c.Get (2), tid);
	  InetSocketAddress remote = InetSocketAddress (Ipv4Address ("255.255.255.255"), 80);
	  source->SetAllowBroadcast (true);
	  source->Connect (remote);

	  Simulator::ScheduleWithContext (source->GetNode ()->GetId (),
                                  Seconds (30.0), &GenerateTraffic,
                                  source, packetSize, numPackets, interPacketInterval);
   */
  /*
  Ptr<Socket> source = Socket::CreateSocket (c.Get (sourceNode), tid);
  // 
  InetSocketAddress remote = InetSocketAddress (i.GetAddress (14, 0), 80);
  source->Connect (remote);
  
  Simulator::Schedule(Seconds (30.0), &GenerateTraffic, source, packetSize, numPackets, interPacketInterval);

  */
  Simulator::Schedule(Seconds (30.0), &SendDataToNeighbours, node_cont.Get(6));

  if (tracing == true)
    {
      AsciiTraceHelper ascii;
      wifiPhy.EnableAsciiAll (ascii.CreateFileStream ("wifi-simple-adhoc-grid.tr"));
      wifiPhy.EnablePcap ("wifi-simple-adhoc-grid", devices);
      // Trace routing tables
      Ptr<OutputStreamWrapper> routingStream = Create<OutputStreamWrapper> ("wifi-simple-adhoc-grid.routes", std::ios::out);
      olsr.PrintRoutingTableAllEvery (Seconds (2), routingStream);
      Ptr<OutputStreamWrapper> neighborStream = Create<OutputStreamWrapper> ("wifi-simple-adhoc-grid.neighbors", std::ios::out);
      olsr.PrintNeighborCacheAllEvery (Seconds (2), neighborStream);

      // To do-- enable an IP-level trace that shows forwarding events only
    }

  // Give OLSR time to converge-- 30 seconds perhaps
  
  // Output what we are doing
  NS_LOG_UNCOND ("Testing from node " << sourceNode << " to " << sinkNode << " with grid distance " << distance);

  Simulator::Stop (Seconds (sym_time));
  AnimationInterface anim ("risenie.xml");
  anim.EnablePacketMetadata ();
  anim.EnableIpv4RouteTracking ("routingtable-wireless.xml", Seconds (0), Seconds (5), Seconds (0.25));

  Simulator::Run ();
  Simulator::Destroy ();

  return 0;
}

