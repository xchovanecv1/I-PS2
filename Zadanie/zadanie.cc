/*
    Vizualizácia: NetAnim 2b (pozn. využite cmd-line-arguments)
 *      --enableNetanim
 *  Reprezentácia merania: Tri grafy (pozn. minimálne 10 bodov merania s vyhodnotením, t.j. odchýlky merania) 3b + zhodnotenie grafu (prečo je to taká závislosť) 2b

 *  Popis merania:
        vhodný vyber ISO OSI — v úvodnom komentári popíšte vhodnosť výberu jednotlivých protokolov … 4b
 *          Vyhody vyuzitia OLSR:
 * 
        volanie časových udalostí  2b
 *          Simulator::Schedule(Seconds (30.0), &SendDataToNeighbours, node_cont.Get(6), "testovacia sprava");

            Simulator::Schedule(Seconds (61.0), &SendDataToNeighbours, node_cont.Get(17), "testovacia sprava2");
            Simulator::Schedule(Seconds (63.0), &SendDataToNeighbours, node_cont.Get(17), "GoLastPos|");
            Simulator::Schedule(Seconds (80.0), &CallNeighbours, node_cont.Get(16));
        volanie udalostí zmenu stavu (atributu modelu) 2b
 *          Simulator::Schedule(Seconds (40.0), &CallNeighbours, node_cont.Get(6));
            Simulator::Schedule(Seconds (41.0), &CallNeighbours, node_cont.Get(12));
            Simulator::Schedule(Seconds (45.0), &CallNeighbours, node_cont.Get(11));
            Simulator::Schedule(Seconds (60.0), &CallNeighbours, node_cont.Get(18));
        zmena v modelu L1  fyzické médium, pohyb, útlm … 3b
 *          - privolanie okolitych dronov do svojho okolia pre dokladnejsi prieskum miesta zaujmu
 *          "/NodeList/---/$ns3::MobilityModel/$ns3::RandomWalk2dMobilityModel/Direction";
 *              -"ns3::UniformRandomVariable[Min=---|Max=---]";
 *          "/NodeList/---/$ns3::MobilityModel/$ns3::RandomWalk2dMobilityModel/Speed";
 *              -"ns3::UniformRandomVariable[Min=---|Max=---]";
        
 *      Implementacia vlastneho broadcast protokolu
 *       - Pomocou OLSR routovacej tabulky si vyhladame susedov, ktory su klasifikovany vo vzdalenosti 1 hopu.
 *       - Kazdy datovy paket obsahuje informacie:
 *          cislo odosielajuceho nodu:cislo spravy:data
 *          -> cislo odosielatela: IDcko daneho nodku ktory vyslal danu spravu.
 *          -> cislo spravy: poradove cislo danej spravy pre dany node
 *       Protokol:
 *          -> kazdy node si drzi posledne ID spravy, ktoru prijal od jedntlivych nodov
 *          -> ak prijmem spravu, ktorej ID je vacsie ako posledne zname ID, spracujem data a preposieam vsetkym mojim susedom
 *          -> ak je ID danej spravy mensie alebo rovnake ako aktualne, dana sprava je zahodena, pretoze prisla od ineho nodu, ktoremu som danu spravu poslal ja     
        zmena v modelu L2-L5 2b
3. Simulácia elektronickej hliadky prevencie poziaru.
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
#include <sstream>      // std::stringstream
#include <algorithm>
#include <math.h> 

using namespace ns3;
using namespace std;

NS_LOG_COMPONENT_DEFINE ("WifiSimpleAdhocGrid");


NodeContainer node_cont;
Ipv4InterfaceContainer ip_container;
unsigned int *node_message_count;
int **node_last_message;
Vector *last_pos;

void StopNode(Ptr<Node> node) {
    
                
                std::stringstream pathdir;
                
                pathdir << "/NodeList/"<< node->GetId() << "/$ns3::MobilityModel/$ns3::RandomWalk2dMobilityModel/Direction";
                
                std::stringstream pathspeed;
                
                pathspeed << "/NodeList/"<< node->GetId() << "/$ns3::MobilityModel/$ns3::RandomWalk2dMobilityModel/Speed";
                
                cout << pathspeed.str() << endl;
                stringstream setval;
                
                setval << "ns3::UniformRandomVariable[Min=0.0|Max=6.283184]";
                
                stringstream setspeed;
                
                setspeed << "ns3::UniformRandomVariable[Min=0.0|Max=5]";
                        
                cout << setval.str() << endl;
                Config::Set(pathdir.str(),StringValue (setval.str()));
                Config::Set(pathspeed.str(),StringValue (setspeed.str()));
                

                Config::Set(pathspeed.str(),StringValue (setspeed.str()));
}

void MoveUAVPos(Ptr<Node> node, Vector toPos) {
    
    Ptr<MobilityModel> sender_model = node->GetObject<MobilityModel>();
      Vector reciever_pos = sender_model->GetPosition();
      
                double dy = toPos.y - reciever_pos.y;
                double dx = toPos.x - reciever_pos.x;
                double e = sqrt((dx*dx) + (dy*dy));

                double speed = 50.0;
                
                double alp = atan2((dy), dx) + 3.14 ;//6.28; //asin (dy/e);
                
                double stop_time = (e/speed) * 0.8;
                
                std::stringstream pathdir;
                
                pathdir << "/NodeList/"<< node->GetId() << "/$ns3::MobilityModel/$ns3::RandomWalk2dMobilityModel/Direction";
                
                std::stringstream pathspeed;
                
                pathspeed << "/NodeList/"<< node->GetId() << "/$ns3::MobilityModel/$ns3::RandomWalk2dMobilityModel/Speed";
                
                cout << pathspeed.str() << endl;
                stringstream setval;
                
                setval << "ns3::UniformRandomVariable[Min="<< alp<< "|Max="<< alp<< "]";
                
                stringstream setspeed;
                
                setspeed << "ns3::UniformRandomVariable[Min=" << speed << "|Max="<< speed <<"]";
                        
                cout << setval.str() << endl;
                Config::Set(pathdir.str(),StringValue (setval.str()));
                Config::Set(pathspeed.str(),StringValue (setspeed.str()));
                cout << "Uhel " << alp << endl;
                
                cout << " Zastaf " << stop_time << endl;
                
                Simulator::Schedule(Seconds (stop_time), &StopNode, node);
}

void DataCallback(Ptr<Node> node, int sender_id, string datas) {
    
      size_t frst = datas.find('|');
      Ptr<MobilityModel> sender_model = node->GetObject<MobilityModel>();
      Vector sender_pos = sender_model->GetPosition();
      int my_id = node->GetId();
      if(frst !=  string::npos) {
          string command = datas.substr(0, frst);
          cout << "Command " << command << endl;
          if(command.compare("SetPos") == 0) {
              
            size_t scnd = datas.find('|',frst+1);
              string x = datas.substr(frst+1, scnd-frst-1);
                string y = datas.substr(scnd+1);
                double pos_x = stod(x);
                double pos_y = stod(y);
            //int sender = stoi(sender_id);
                /*
                Ptr<MobilityModel> mdl = node_cont.Get(sender_id)->GetObject<MobilityModel>();
                Vector reciever_pos = mdl->GetPosition();
                MoveUAVPos(node_cont.Get(sender_id), sender_pos);
*/
                Ptr<MobilityModel> mdl = node_cont.Get(sender_id)->GetObject<MobilityModel>();
                Vector reciever_pos = mdl->GetPosition();
                
                cout << "Sender " << sender_pos.x << " " << sender_pos.y << endl;
                cout << "Recv " << reciever_pos.x << " " << reciever_pos.y << endl;
                
                double dy = sender_pos.y - reciever_pos.y;
                double dx = sender_pos.x - reciever_pos.x;
                double e = sqrt((dx*dx) + (dy*dy));
                
                last_pos[my_id].x = reciever_pos.x;
                last_pos[my_id].y = reciever_pos.y;
                last_pos[my_id].z = reciever_pos.z;
                
                double speed = 50.0;
                
                double alp = atan2((dy), dx) + 3.14 ;//6.28; //asin (dy/e);
                
                double stop_time = (e/speed) * 0.8;
                
                std::stringstream pathdir;
                
                pathdir << "/NodeList/"<< node->GetId() << "/$ns3::MobilityModel/$ns3::RandomWalk2dMobilityModel/Direction";
                
                std::stringstream pathspeed;
                
                pathspeed << "/NodeList/"<< node->GetId() << "/$ns3::MobilityModel/$ns3::RandomWalk2dMobilityModel/Speed";
                
                cout << pathspeed.str() << endl;
                stringstream setval;
                
                setval << "ns3::UniformRandomVariable[Min="<< alp<< "|Max="<< alp<< "]";
                
                stringstream setspeed;
                
                setspeed << "ns3::UniformRandomVariable[Min=" << speed << "|Max="<< speed <<"]";
                        
                cout << setval.str() << endl;
                Config::Set(pathdir.str(),StringValue (setval.str()));
                Config::Set(pathspeed.str(),StringValue (setspeed.str()));
                cout << "Uhel " << alp << endl;
                
                cout << " Zastaf " << stop_time << endl;
                
                Simulator::Schedule(Seconds (stop_time), &StopNode, node);
                 
                //mdl->DoSetPosition(Vector (pos_x, pos_y, 0));
                //mdl->DoWalk(Seconds(2));
                /*
                    MobilityHelper mobility_sink;
                    Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();
                    positionAlloc->Add (Vector (pos_x, pos_y, 0.0));
                    mobility_sink.SetPositionAllocator (positionAlloc);
                    mobility_sink.SetMobilityModel ("ns3::ConstantVelocityMobilityModel");
                    mobility_sink.Install (node);
                    node -> GetObject<ConstantVelocityMobilityModel>() -> SetVelocity(Vector(20.0, 20.0, 0.0));  
                */
              cout << "Setting position for node " << node->GetId() << "["<<x << " " << y << "]" << endl;
              
          } else if(command.compare("GoLastPos") == 0) {
              
              if(last_pos[my_id].x == 0 && last_pos[my_id].y== 0 && last_pos[my_id].z == 0) return;
              cout << "Idem dom" << endl;
                double dy = sender_pos.y - last_pos[my_id].y;
                double dx = sender_pos.x - last_pos[my_id].x;
                double e = sqrt((dx*dx) + (dy*dy));

                double speed = 50.0;
                
                double alp = atan2((dy), dx) + 3.14 ;//6.28; //asin (dy/e);
                
                double stop_time = (e/speed) * 0.8;
                
                std::stringstream pathdir;
                
                pathdir << "/NodeList/"<< node->GetId() << "/$ns3::MobilityModel/$ns3::RandomWalk2dMobilityModel/Direction";
                
                std::stringstream pathspeed;
                
                pathspeed << "/NodeList/"<< node->GetId() << "/$ns3::MobilityModel/$ns3::RandomWalk2dMobilityModel/Speed";
                
                cout << pathspeed.str() << endl;
                stringstream setval;
                
                setval << "ns3::UniformRandomVariable[Min="<< alp<< "|Max="<< alp<< "]";
                
                stringstream setspeed;
                
                setspeed << "ns3::UniformRandomVariable[Min=" << speed << "|Max="<< speed <<"]";
                        
                cout << setval.str() << endl;
                Config::Set(pathdir.str(),StringValue (setval.str()));
                Config::Set(pathspeed.str(),StringValue (setspeed.str()));
                cout << "Uhel " << alp << endl;
                
                cout << " Zastaf " << stop_time << endl;
                
                Simulator::Schedule(Seconds (stop_time), &StopNode, node);
          }
      }
      
    
    cout << datas << " from " << sender_id << endl;
}

static void SendDataToNeighbours(Ptr< Node > sNode, uint8_t * data, int msg_id, int sndr_id);
static void SendDataToNeighbours(Ptr< Node > sNode, string data);

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

void CallNeighbours(Ptr<Node> node) {
    
    Ptr<MobilityModel> mdl = node->GetObject<MobilityModel>();
    Vector pos = mdl->GetPosition();
    TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
    
    cout << "Som tu: " << pos.x << " " << pos.y << ";" << endl;
    
    int node_id = node->GetId();
    
    //node_message_count[node_id]++;
    
    std::stringstream ss;
    
    ss << node->GetId() << ":" << 0 << ":" << "SetPos|" << to_string(pos.x) << "|" << to_string(pos.y);
    
    string dta = ss.str();
    cout << dta << endl;
    Ptr<Socket> source = Socket::CreateSocket (node, tid);
    InetSocketAddress remote = InetSocketAddress (Ipv4Address ("255.255.255.255"), 80);
    source->SetAllowBroadcast (true);
    source->Connect (remote);
    source->Send (Create<Packet> ((uint8_t*)dta.c_str(), dta.length()+1));
    source->Close();
    
    
    //->SetPosition
}


void GoHome(Ptr<Node> node) {
    
    SendDataToNeighbours(node, "GoLastPos");
    
}

void ReceivePacket (Ptr<Socket> socket)
{
    Ptr<Packet> rcv;
 
    Ptr<Node> trt = socket->GetNode();
    int node_id = trt->GetId();
    //cout << trt->GetId() << endl;
    TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
  while (( rcv = socket->Recv ()))
    {
      
      uint32_t size = rcv->GetSize();
      char * data = (char*)malloc(size);
      /*cout << rcv->ToString() << " " << rcv->GetSize()  << endl;
      NS_LOG_UNCOND ("Received one packet!");
      cout << "Bravcova pata" << endl;*/
      rcv->CopyData ((uint8_t*)data, size);
      //cout << "Magian: " << data << endl;
      
      string datas(data);
      int frst = datas.find(':');
      int scnd = datas.find(':',frst+1);
      
      string sender_id = datas.substr(0, frst);
      string msg_id = datas.substr(frst+1, scnd-frst-1);
      string pckt_data = datas.substr(scnd+1);
      
      int sender = stoi(sender_id);
      int message_id = stoi(msg_id);
      
      char* snd_data = (char*)malloc(pckt_data.length()+1);
      strcpy(snd_data, &(datas[scnd+1]));
      
      //cout << sender <<" " <<  message_id << " " << pckt_data << endl;
      //cout  << message_id << " a " << node_last_message[node_id][sender] << endl;
      if(message_id > node_last_message[node_id][sender]) { // resending to nbrs
          node_last_message[node_id][sender] = message_id;
          DataCallback(trt, sender, pckt_data);
          SendDataToNeighbours(trt, (uint8_t*) snd_data, message_id, sender);
          cout << "Nova sprava od: " << sender<< " Ja: " << node_id << " Spravaid:" << message_id << " SPrava: " << pckt_data << endl;
      } else if (message_id == 0){
          DataCallback(trt, sender, pckt_data);
      }
      free(snd_data);
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
      //Simulator::Schedule (pktInterval, &GenerateTraffic, socket, pktSize,pktCount - 1, pktInterval);
 /*   }
  else
    {
      socket->Close ();
    }*/
}
static void SendDataToNeighbours(Ptr< Node > sNode, string data) {
    uint8_t* dta = (uint8_t*)malloc(data.length()+1);
    memset(dta, 0, data.length()+1);
    sprintf((char*)dta, "%s", data.c_str());
    SendDataToNeighbours(sNode,(uint8_t*)dta,-1,-1);
    free(dta);
}

static void SendDataToNeighbours(Ptr< Node > sNode, uint8_t * data, int msg_id = -1, int sndr_id = -1) {
  
    Ptr<Ipv4> stack = sNode->GetObject<Ipv4> ();
  Ptr<Ipv4RoutingProtocol> rp_Gw = (stack->GetRoutingProtocol ());
  Ptr<Ipv4ListRouting> lrp_Gw = DynamicCast<Ipv4ListRouting> (rp_Gw);

  Ptr<olsr::RoutingProtocol> olsrrp_Gw;

  int sender_id = sNode->GetId();
  
  if(msg_id < 0) {
    node_message_count[sender_id]++;  
    msg_id = node_message_count[sender_id];
  }
  
  if(sndr_id > 0) {
    sender_id = sndr_id;
  }
  
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


        uint8_t* packet = (uint8_t*)malloc(600);
        
        sprintf((char*)packet, "%d:%d:%s",sender_id,msg_id,data);
        source->Send (Create<Packet> (packet, 600));
        
        //GenerateDataN(source, msg_id, data, strlen(data));
        source->Close();
      }
      
      //cout << bf.destAddr << ": " << bf.distance << endl;
  }

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
 

void TableChanged(string path, uint32_t val) {
     std::stringstream test(path);
    std::string segment;
    std::vector<std::string> seglist;

    while(std::getline(test, segment, '/'))
    {
       seglist.push_back(segment);
    }
    
    int nodeID = stoi(seglist[2]);
    Time now = Simulator::Now();
    //Avoid initial routing
    if(val == 0 && (now > Seconds(30.0))) {
        MoveUAVPos(node_cont.Get(nodeID), Vector(1000.0,1000.0,0));
    }
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
  
  bool enableNetanim = true;
  float helloInterval = 2.0;
  
  uint32_t sym_time = 100;
  uint32_t send_time = 30;
  uint32_t send_time_i = 5;

  uint32_t m_maxX = (distance) * numNodesInRow;
  uint32_t m_minX = 0;
  uint32_t m_maxY = (distance) * numNodesInRow;
  uint32_t m_minY = 0;
  
  CommandLine cmd;
 cmd.AddValue ("enableNetanim", "Vystupna animacia pre Netanim", enableNetanim);
 cmd.AddValue ("helloInterval", "Interval hello paketov (OLSR)", helloInterval);
 cmd.Parse (argc, argv);
  
  node_message_count = (unsigned int *)new int[numNodes];
  memset(node_message_count, 0, numNodes*(sizeof(unsigned int)));
  
  last_pos = new Vector[numNodes];
  for(int i=0; i < numNodes; i++) {
      last_pos[i].x = 0;
      last_pos[i].y = 0;
      last_pos[i].z = 0;
  }
  
  node_last_message = new int*[numNodes];
    for(int i = 0; i < numNodes; ++i){
        node_last_message[i] = new int[numNodes];
        for(int d = 0; d < numNodes; ++d){
            node_last_message[i][d] = 0;
        }
    }
    /*
  for(int i = 0; i < numNodes; i++) {
      node_last_message[i] = (unsigned int *)malloc(numNodes);
      memset(node_last_message[i], 0, numNodes*(sizeof(unsigned int)));
  }
  */
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
                            "Speed", StringValue ("ns3::UniformRandomVariable[Min=0.0|Max=5]"),
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
  Simulator::Schedule(Seconds (30.0), &SendDataToNeighbours, node_cont.Get(6), "testovacia sprava");
  
  Simulator::Schedule(Seconds (40.0), &CallNeighbours, node_cont.Get(6));
  Simulator::Schedule(Seconds (41.0), &CallNeighbours, node_cont.Get(12));
  
  Simulator::Schedule(Seconds (45.0), &CallNeighbours, node_cont.Get(11));
  
  Simulator::Schedule(Seconds (60.0), &CallNeighbours, node_cont.Get(18));
   //Simulator::Schedule(Seconds (61.0), &SendDataToNeighbours, node_cont.Get(17), "testovacia sprava2");
  
  Simulator::Schedule(Seconds (63.0), &SendDataToNeighbours, node_cont.Get(17), "GoLastPos|");
  
  Simulator::Schedule(Seconds (80.0), &CallNeighbours, node_cont.Get(16));
 
  // V pripade ak je cely konstruktor v If, Netanim negeneruje ziadne udaje, preto je defaultne vypnuty a nasledne v pripade pozadovaneho netanimu nastavime koniec sym casu
  AnimationInterface anim ("zadanie.xml");
  anim.SetStopTime (Seconds(0));
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
  if(enableNetanim) {
    anim.SetStopTime (Seconds(sym_time));
    anim.EnablePacketMetadata ();
    anim.EnableIpv4RouteTracking ("routingtable-wireless.xml", Seconds (0), Seconds (5), Seconds (0.25));
    
  }
  
  Config::Connect("/NodeList/*/$ns3::olsr::RoutingProtocol/RoutingTableChanged", MakeCallback(&TableChanged));
  
  Simulator::Schedule (Seconds (sym_time/4), &Config::Set, "/NodeList/*/$ns3::olsr::RoutingProtocol/HelloInterval", TimeValue(Seconds(helloInterval)));
  
  Simulator::Run ();
  Simulator::Destroy ();

  return 0;
}

