#include <iostream>
#include <vector>
#include <fstream>
#include <queue>
#include <ctime>
#include <numeric>
#include <sstream>

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/applications-module.h"
#include "ns3/mobility-module.h"
#include "ns3/wifi-module.h"
#include "ns3/olsr-module.h"
#include "ns3/aodv-module.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/netanim-module.h"
#include "myapp.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("mladoniczky-ps2");

/*Cal mean from all dropped bytes*/
uint64_t meanDroppedBytes(std::vector<uint64_t> dropped){
	if(dropped.size() == 0) return 0;
	uint64_t sum = std::accumulate(dropped.begin(),dropped.end(),0);
	return (uint64_t)((double)sum/(double)dropped.size());
}

int main(int argc, char* argv[]){
	srand(time(NULL));
	std::clock_t runTime = std::clock();

	/*Params*/
	std::string phyMode("DsssRate1Mbps");
	double sim_duration = 100.;
	std::string author = "mladoniczky";

	int nnodes = 10;
	uint32_t packetSize = 1024; //Bytes
	bool generatePcap = false;
	bool saveDelays = false;
	bool saveLossRates = false;
	bool saveUsefulDataBytes = false;
	std::string outputDir = ".";
	bool printAll = false;
	std::string changing = "";

	CommandLine cmd;
	cmd.AddValue("nodes","Number of nodes",nnodes);
	cmd.AddValue("packet","Size of packet in bytes",packetSize);
	cmd.AddValue("pcap","Generate Pcap files for wireshark",generatePcap);
	cmd.AddValue("sdelays","Save delays of connections into file",saveDelays);
	cmd.AddValue("sloss","Save packet loss rates of connnections into file",saveLossRates);
	cmd.AddValue("suseful","Save ratio of mean of useful data and all tranfered packets of connnections into file",saveUsefulDataBytes);
	cmd.AddValue("outdir","Directory where output files will be saved",outputDir);
	cmd.AddValue("printall","Print every results of simulation into console",printAll);
	cmd.AddValue("changing","Which paramater is changing from simulation to simulation",changing);
	cmd.Parse(argc, argv);

	// Fix non-unicast data rate to be the same as that of unicast
  	Config::SetDefault ("ns3::WifiRemoteStationManager::NonUnicastMode", StringValue (phyMode));

	/*Create Nodes*/
	//std::cout << "Creating nodes" << std::endl;
	NodeContainer nodeContainer;
	nodeContainer.Create(nnodes);

	/*WiFi setup*/
	//std::cout << "Setting up Wifi" << std::endl;
	WifiHelper wifi;
	YansWifiPhyHelper wifiPhy = YansWifiPhyHelper::Default();
	wifiPhy.SetPcapDataLinkType(YansWifiPhyHelper::DLT_IEEE802_11);

	YansWifiChannelHelper wifiChannel;
	wifiChannel.SetPropagationDelay("ns3::RandomPropagationDelayModel"); //random delay fo signal
	wifiChannel.AddPropagationLoss("ns3::LogDistancePropagationLossModel"); //losses are equal three times of log of distance
	//nastavenia
	// wifiPhy.Set ("TxPowerStart", DoubleValue(33));
	// wifiPhy.Set ("TxPowerEnd", DoubleValue(33));
	// wifiPhy.Set ("TxPowerLevels", UintegerValue(1));
	// wifiPhy.Set ("TxGain", DoubleValue(0));
	// wifiPhy.Set ("RxGain", DoubleValue(0));
	// wifiPhy.Set ("EnergyDetectionThreshold", DoubleValue(-61.8));
	// wifiPhy.Set ("CcaMode1Threshold", DoubleValue(-64.8));
	//wifiPhy.Set("RxGain",DoubleValue(-10));
	wifiPhy.SetChannel(wifiChannel.Create());

	/*Add non-QoS upper mac*/
	//std::cout << "Adding non-QoS upper mac" << std::endl;
	NqosWifiMacHelper wifiMac = NqosWifiMacHelper::Default();
	wifi.SetStandard(WIFI_PHY_STANDARD_80211b);
	wifi.SetRemoteStationManager("ns3::ConstantRateWifiManager",
								"DataMode", StringValue(phyMode),
								"ControlMode", StringValue(phyMode));
	wifiMac.SetType("ns3::AdhocWifiMac");
	NetDeviceContainer devices = wifi.Install(wifiPhy, wifiMac, nodeContainer);

	// /*OLSR*/
	//std::cout << "Setting up OLSR" << std::endl;
	OlsrHelper olsr;
	Ipv4ListRoutingHelper list;
	list.Add(olsr, 10);
	/*AODV*/
	// std::cout << "Setting up AODV" << std::endl;
	// AodvHelper aodv;
	// Ipv4ListRoutingHelper list;
	// list.Add(aodv,10);

	/*Setup internet stack*/
	//std::cout << "Setting up Internet stack" << std::endl;
	InternetStackHelper internet;
	internet.SetRoutingHelper(list);
	internet.Install(nodeContainer);

	/*Setup Ip addresses*/
	//std::cout << "Assigning Ip addresses" << std::endl;
	Ipv4AddressHelper ipv4;
	ipv4.SetBase("10.1.1.0","255.255.255.0");
	Ipv4InterfaceContainer ifcont = ipv4.Assign(devices);

	/*Set Mobility for all nodes*/
	//std::cout << "Setting up mobility model for nodes" << std::endl;
	MobilityHelper mobility;
	mobility.SetPositionAllocator("ns3::RandomRectanglePositionAllocator",
									"X",StringValue("ns3::UniformRandomVariable[Min=0|Max=250]"),//movement boundaries
									"Y",StringValue("ns3::UniformRandomVariable[Min=0|Max=250]"));
	mobility.SetMobilityModel("ns3::RandomWalk2dMobilityModel","Bounds", //random move with random speed
								RectangleValue(Rectangle(-1,251,-1,251)),
								"Distance",DoubleValue(20.0));
	mobility.Install(nodeContainer);

	/*Pick nodes to communicate*/
	//std::cout << "Picking random nodes" << std::endl;
	std::deque<int> deq;
	for(int i=0; i<nnodes; i++){
		deq.push_front(i);
	}
	random_shuffle(deq.begin(), deq.end());
	std::queue<int> que(deq);
	int N1, N2, N4, N5, N6, N7, N8, N9, N10, N11;
	N1 = que.front();que.pop();
	N2 = que.front();que.pop();
	N4 = que.front();que.pop();
	N5 = que.front();que.pop();
	N6 = que.front();que.pop();
	N7 = que.front();que.pop();
	N8 = que.front();que.pop();
	N9 = que.front();que.pop();
	N10 = que.front();que.pop();
	N11 = que.front();que.pop();

	/*UDP Setup*/
	//std::cout << "Setting up UDP communication" << std::endl;
	uint16_t sinkPort = 6;
	uint32_t npackets = 10485760.0/(double)packetSize; //10485760B = 10MB

	std::cout << "Number of nodes: " << nnodes << std::endl;
	std::cout << "Packet size: " << packetSize << std::endl;
	std::cout << "Number of packets: " << npackets << std::endl;
	std::cout << "Created connections: " << std::endl;

	/*Nodes communication*/
	/**N1 -> N2 ; delay=10s**/
	std::cout << "N" << (N1+1) << " -> N" << (N2+1) << std::endl;
	Address sinkAddressN2(InetSocketAddress(ifcont.GetAddress(N2),sinkPort));
	PacketSinkHelper packetSinkHelperN2("ns3::UdpSocketFactory",
									InetSocketAddress(Ipv4Address::GetAny(),
									sinkPort));
	ApplicationContainer sinkAppsN2 = packetSinkHelperN2.Install(nodeContainer.Get(N2));
	sinkAppsN2.Start(Seconds(0.));
	sinkAppsN2.Stop(Seconds(sim_duration));

	Ptr<Socket> ns3UdpSocketN1 = Socket::CreateSocket(nodeContainer.Get(N1),
													UdpSocketFactory::GetTypeId());
	Ptr<MyApp> appN1 = CreateObject<MyApp>();
	appN1->Setup(ns3UdpSocketN1, sinkAddressN2, packetSize, npackets, DataRate("250Kbps"));
	nodeContainer.Get(N1)->AddApplication(appN1);
	appN1->SetStartTime(Seconds(10.));
	appN1->SetStopTime(Seconds(sim_duration));

	/**N4 -> N5 ; delay=12s**/
	std::cout << "N" << (N4+1) << " -> N" << (N5+1) << std::endl;
	Address sinkAddressN5(InetSocketAddress(ifcont.GetAddress(N5),sinkPort));
	PacketSinkHelper packetSinkHelperN5("ns3::UdpSocketFactory",
									InetSocketAddress(Ipv4Address::GetAny(),
									sinkPort));
	ApplicationContainer sinkAppsN5 = packetSinkHelperN5.Install(nodeContainer.Get(N5));
	sinkAppsN5.Start(Seconds(0.));
	sinkAppsN5.Stop(Seconds(sim_duration));

	Ptr<Socket> ns3UdpSocketN4 = Socket::CreateSocket(nodeContainer.Get(N4),
													UdpSocketFactory::GetTypeId());
	Ptr<MyApp> appN4 = CreateObject<MyApp>();
	appN4->Setup(ns3UdpSocketN4, sinkAddressN5, packetSize, npackets, DataRate("250Kbps"));
	nodeContainer.Get(N4)->AddApplication(appN4);
	appN4->SetStartTime(Seconds(12.));
	appN4->SetStopTime(Seconds(sim_duration));

	/**N6 -> N7 ; delay=15s**/
	std::cout << "N" << (N6+1) << " -> N" << (N7+1) << std::endl;
	Address sinkAddressN7(InetSocketAddress(ifcont.GetAddress(N7),sinkPort));
	PacketSinkHelper packetSinkHelperN7("ns3::UdpSocketFactory",
									InetSocketAddress(Ipv4Address::GetAny(),
									sinkPort));
	ApplicationContainer sinkAppsN7 = packetSinkHelperN7.Install(nodeContainer.Get(N7));
	sinkAppsN7.Start(Seconds(0.));
	sinkAppsN7.Stop(Seconds(sim_duration));

	Ptr<Socket> ns3UdpSocketN6 = Socket::CreateSocket(nodeContainer.Get(N6),
													UdpSocketFactory::GetTypeId());
	Ptr<MyApp> appN6 = CreateObject<MyApp>();
	appN6->Setup(ns3UdpSocketN6, sinkAddressN7, packetSize, npackets, DataRate("250Kbps"));
	nodeContainer.Get(N6)->AddApplication(appN6);
	appN6->SetStartTime(Seconds(15.));
	appN6->SetStopTime(Seconds(sim_duration));

	/**N8 -> N9 ; delay=16s**/
	std::cout << "N" << (N8+1) << " -> N" << (N9+1) << std::endl;
	Address sinkAddressN9(InetSocketAddress(ifcont.GetAddress(N9),sinkPort));
	PacketSinkHelper packetSinkHelperN9("ns3::UdpSocketFactory",
									InetSocketAddress(Ipv4Address::GetAny(),
									sinkPort));
	ApplicationContainer sinkAppsN9 = packetSinkHelperN9.Install(nodeContainer.Get(N9));
	sinkAppsN9.Start(Seconds(0.));
	sinkAppsN9.Stop(Seconds(sim_duration));

	Ptr<Socket> ns3UdpSocketN8 = Socket::CreateSocket(nodeContainer.Get(N8),
													UdpSocketFactory::GetTypeId());
	Ptr<MyApp> appN8 = CreateObject<MyApp>();
	appN8->Setup(ns3UdpSocketN8, sinkAddressN9, packetSize, npackets, DataRate("250Kbps"));
	nodeContainer.Get(N8)->AddApplication(appN8);
	appN8->SetStartTime(Seconds(16.));
	appN8->SetStopTime(Seconds(sim_duration));

	/**N10 -> N11 ; delay=21s**/
	std::cout << "N" << (N10+1) << " -> N" << (N11+1) << std::endl;
	Address sinkAddressN11(InetSocketAddress(ifcont.GetAddress(N11),sinkPort));
	PacketSinkHelper packetSinkHelperN11("ns3::UdpSocketFactory",
									InetSocketAddress(Ipv4Address::GetAny(),
									sinkPort));
	ApplicationContainer sinkAppsN11 = packetSinkHelperN11.Install(nodeContainer.Get(N11));
	sinkAppsN11.Start(Seconds(0.));
	sinkAppsN11.Stop(Seconds(sim_duration));

	Ptr<Socket> ns3UdpSocketN10 = Socket::CreateSocket(nodeContainer.Get(N10),
													UdpSocketFactory::GetTypeId());
	Ptr<MyApp> appN10 = CreateObject<MyApp>();
	appN10->Setup(ns3UdpSocketN10, sinkAddressN11, packetSize, npackets, DataRate("250Kbps"));
	nodeContainer.Get(N10)->AddApplication(appN10);
	appN10->SetStartTime(Seconds(21.));
	appN10->SetStopTime(Seconds(sim_duration));

	/*NetAnim setup*/
	//std::cout << "Setting up NetAnim" << std::endl;
	std::string netAnimOutput = outputDir+"/netanim/"+author+"-netanim-"+changing;
	std::string routingOutput = outputDir+"/netanim/"+author+"-routingtable-"+changing;
	std::string flowmonOutput = outputDir+"/flowmon/"+author+"-"+changing;
	std::ostringstream ss;
	if(std::strcmp(changing.c_str(),"nodes") == 0){
		ss << nnodes;
		netAnimOutput += "-"+ss.str();
		routingOutput += "-"+ss.str();
		flowmonOutput += "-"+ss.str();
	}
	else if(std::strcmp(changing.c_str(),"packets") == 0){
		ss << npackets;
		netAnimOutput += "-"+ss.str();
		routingOutput += "-"+ss.str();
		flowmonOutput += "-"+ss.str();
	} else {
		std::cout << "Nothing changed" << std::endl;
	}
	netAnimOutput += ".xml";
	routingOutput += ".xml";
	flowmonOutput += ".flowmon";

	AnimationInterface anim(netAnimOutput);
	for(uint32_t i=0; i<nodeContainer.GetN(); i++){
		std::ostringstream stringStream;
		stringStream << "N" << i;
		anim.UpdateNodeDescription(nodeContainer.Get(i), stringStream.str());
		int R = rand()%256;
		int G = rand()%256;
		int B = rand()%256;
		anim.UpdateNodeColor(nodeContainer.Get(i),R,G,B);
	}
	anim.EnablePacketMetadata();
	anim.EnableIpv4RouteTracking(routingOutput,Seconds(0),
								Seconds(5),Seconds(0.25));
	anim.EnableWifiMacCounters(Seconds(0),Seconds(10));
	anim.EnableWifiPhyCounters(Seconds(0),Seconds(10));

	/*Generate Pcap file if allowed*/
	if(generatePcap){
		wifiPhy.EnablePcap(outputDir+"/pcap/"+author+"-"+changing,devices);
	}

	/*Flowmonitor setup*/
	//std::cout << "Setting up FlowMonitor" << std::endl;
	FlowMonitorHelper flowmon;
	Ptr<FlowMonitor> monitor = flowmon.InstallAll();

	/*Start Simulation*/
	//std::cout << "Running simulation" << std::endl;
	Simulator::Stop(Seconds(sim_duration));
	Simulator::Run();

	/*Pick stats from FlowMonitor*/
	std::ofstream lossFile((outputDir+"/data/loss-rates.dat").c_str(), std::ofstream::app);
	std::ofstream delayFile((outputDir+"/data/delays.dat").c_str(), std::ofstream::app);
	std::ofstream usefulFile((outputDir+"/data/useful-data.dat").c_str(), std::ofstream::app);

	//std::cout << "Getting stats from FlowMonitor" << std::endl;
	monitor->CheckForLostPackets();
	Ptr<Ipv4FlowClassifier> ipClassifier = DynamicCast<Ipv4FlowClassifier>(flowmon.GetClassifier());
	std::map<FlowId, FlowMonitor::FlowStats> stats = monitor->GetFlowStats();
	for(std::map<FlowId, FlowMonitor::FlowStats>::const_iterator it = stats.begin(); it!=stats.end(); it++){
		Ipv4FlowClassifier::FiveTuple t = ipClassifier->FindFlow(it->first);

		/*Calc needed stats*/
		double packetLossRate = ((double)it->second.lostPackets/(double)(it->second.lostPackets + it->second.rxPackets))*100;
		double meanDelay = (it->second.delaySum.GetMilliSeconds()/(double)it->second.rxPackets);
		double usefulData = ((double)(it->second.txBytes - meanDroppedBytes(it->second.bytesDropped))/(double)it->second.txBytes)*100;

		/*Save stats*/
		if(saveLossRates){
			lossFile << nnodes << " " << packetLossRate << std::endl;
		}
		if(saveDelays){
			delayFile << nnodes << " " << meanDelay << std::endl;
		}
		if(saveUsefulDataBytes){
			usefulFile << packetSize << " " << usefulData << std::endl;
		}
		if(printAll){
			std::cout << "FlowId: " << it->first << " Src-Addr:" << t.sourceAddress << " Dst-Addr:" << t.destinationAddress << std::endl;
			std::cout << "Send packets = " << it->second.txPackets << std::endl;
			std::cout << "Received packets = " << it->second.rxPackets << std::endl;
			std::cout << "Throughput = " << it->second.rxBytes*8.0 / (it->second.timeLastRxPacket.GetSeconds() - it->second.timeFirstTxPacket.GetSeconds())/1024 << " Kbps" << std::endl;			
			std::cout << "Packet Loss Rate = " << packetLossRate << "\%" << std::endl;
			std::cout << "Packet Delay Sum = " << it->second.delaySum.As(Time::S) << std::endl;		
			std::cout << "Mean Delay = " << meanDelay << "ms" << std::endl;
			std::cout << "Dropped Bytes = " << meanDroppedBytes(it->second.bytesDropped) << std::endl;
			std::cout << "Mean useful bytes = " << usefulData << "\%" << std::endl;			
		}
	}
	monitor->SerializeToXmlFile(flowmonOutput,true,true);

	Simulator::Destroy();

	lossFile.close();
	delayFile.close();
	usefulFile.close();
	std::cout << "Generated data saved in file in "+outputDir+" directory" << std::endl;
	std::cout << "Finished in: " << (double)(std::clock() - runTime)/(double)CLOCKS_PER_SEC << "s" << std::endl;
	return 0;
}