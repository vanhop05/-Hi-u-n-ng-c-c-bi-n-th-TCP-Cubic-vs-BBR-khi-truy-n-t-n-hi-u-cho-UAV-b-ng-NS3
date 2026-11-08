#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/wifi-module.h"
#include "ns3/applications-module.h"

#include <fstream>
#include <iostream>
#include <string>

using namespace ns3;

std::ofstream g_metricStream;
uint64_t g_lastTotalRx = 0;
uint32_t g_currentCwnd = 0;

static void
CwndChange(uint32_t oldCwnd, uint32_t newCwnd)
{
    g_currentCwnd = newCwnd;
}

static void
TraceCwnd()
{
    Config::ConnectWithoutContext("/NodeList/1/$ns3::TcpL4Protocol/SocketList/0/CongestionWindow",
                                  MakeCallback(&CwndChange));
}

static void
SampleMetrics(Ptr<Node> gcsNode, Ptr<Node> uavNode, Ptr<PacketSink> sink, double interval)
{
    double now = Simulator::Now().GetSeconds();

    Ptr<MobilityModel> gcsMob = gcsNode->GetObject<MobilityModel>();
    Ptr<MobilityModel> uavMob = uavNode->GetObject<MobilityModel>();
    double distance = gcsMob->GetDistanceFrom(uavMob);

    uint64_t currentTotalRx = sink->GetTotalRx();
    double throughput = (currentTotalRx - g_lastTotalRx) * 8.0 / (interval * 1000000.0);
    g_lastTotalRx = currentTotalRx;

    double cwndPkts = g_currentCwnd / 1448.0;

    g_metricStream << now << "," << distance << "," << throughput << "," << cwndPkts << std::endl;

    Simulator::Schedule(Seconds(interval), &SampleMetrics, gcsNode, uavNode, sink, interval);
}

int
main(int argc, char* argv[])
{
    std::string transportProt = "TcpCubic";
    double uavSpeed = 10.0;                 // 10 m/s
    double simTime = 30.0;
    double altitude = 20.0;                 // 20m altitude
    std::string prefix = "uav_results";

    CommandLine cmd(__FILE__);
    cmd.AddValue("transportProt", "Giao thuc TCP (TcpCubic hoac TcpBbr)", transportProt);
    cmd.AddValue("uavSpeed", "Van toc bay (m/s)", uavSpeed);
    cmd.AddValue("simTime", "Thoi gian mo phong (s)", simTime);
    cmd.AddValue("altitude", "Do cao bay (m)", altitude);
    cmd.AddValue("prefix", "Tien to file ket qua", prefix);
    cmd.Parse(argc, argv);

    Config::SetDefault("ns3::TcpL4Protocol::SocketType", StringValue("ns3::" + transportProt));
    Config::SetDefault("ns3::TcpSocket::SegmentSize", UintegerValue(1448));

    NodeContainer nodes;
    nodes.Create(2);

    MobilityHelper mobility;
    Ptr<ListPositionAllocator> posAlloc = CreateObject<ListPositionAllocator>();
    posAlloc->Add(Vector(0.0, 0.0, 0.0));             // GCS
    posAlloc->Add(Vector(5.0, 0.0, altitude));        // UAV start at 5m
    mobility.SetPositionAllocator(posAlloc);

    mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    mobility.Install(nodes.Get(0));

    mobility.SetMobilityModel("ns3::ConstantVelocityMobilityModel");
    mobility.Install(nodes.Get(1));
    nodes.Get(1)->GetObject<ConstantVelocityMobilityModel>()->SetVelocity(Vector(uavSpeed, 0.0, 0.0));

    // WiFi Configuration
    YansWifiChannelHelper channel;
    channel.SetPropagationDelay("ns3::ConstantSpeedPropagationDelayModel");
    // Line-of-sight Air-to-ground model (exponent 2.1)
    channel.AddPropagationLoss("ns3::LogDistancePropagationLossModel",
                               "Exponent", DoubleValue(2.2),
                               "ReferenceDistance", DoubleValue(1.0),
                               "ReferenceLoss", DoubleValue(40.0));

    YansWifiPhyHelper phy;
    phy.SetChannel(channel.Create());
    // UAV transmitter power 20 dBm (100 mW)
    phy.Set("TxPowerStart", DoubleValue(20.0));
    phy.Set("TxPowerEnd", DoubleValue(20.0));

    WifiHelper wifi;
    wifi.SetStandard(WIFI_STANDARD_80211a);
    wifi.SetRemoteStationManager("ns3::AarfWifiManager");

    WifiMacHelper mac;
    mac.SetType("ns3::AdhocWifiMac");

    NetDeviceContainer devices = wifi.Install(phy, mac, nodes);

    InternetStackHelper internet;
    internet.Install(nodes);

    Ipv4AddressHelper ipv4;
    ipv4.SetBase("192.168.1.0", "255.255.255.0");
    Ipv4InterfaceContainer interfaces = ipv4.Assign(devices);

    uint16_t port = 8080;
    Address sinkAddress(InetSocketAddress(Ipv4Address::GetAny(), port));
    PacketSinkHelper sinkHelper("ns3::TcpSocketFactory", sinkAddress);
    ApplicationContainer sinkApp = sinkHelper.Install(nodes.Get(0));
    sinkApp.Start(Seconds(0.0));
    sinkApp.Stop(Seconds(simTime));

    Address remoteAddress(InetSocketAddress(interfaces.GetAddress(0), port));
    BulkSendHelper sourceHelper("ns3::TcpSocketFactory", remoteAddress);
    sourceHelper.SetAttribute("MaxBytes", UintegerValue(0));
    ApplicationContainer sourceApp = sourceHelper.Install(nodes.Get(1));
    sourceApp.Start(Seconds(1.0));
    sourceApp.Stop(Seconds(simTime));

    std::string csvFileName = prefix + "_" + transportProt + "_metrics.csv";
    g_metricStream.open(csvFileName.c_str());
    g_metricStream << "Time,Distance_m,Throughput_Mbps,Cwnd_Packets\n";

    g_lastTotalRx = 0;
    g_currentCwnd = 0;

    Simulator::Schedule(Seconds(1.0001), &TraceCwnd);
    Ptr<PacketSink> sink = DynamicCast<PacketSink>(sinkApp.Get(0));
    Simulator::Schedule(Seconds(1.5), &SampleMetrics, nodes.Get(0), nodes.Get(1), sink, 0.5);

    Simulator::Stop(Seconds(simTime + 0.1));
    Simulator::Run();

    uint64_t totalBytes = sink->GetTotalRx();
    double avgThroughput = (totalBytes * 8.0) / ((simTime - 1.0) * 1000000.0);
    double finalDist = nodes.Get(0)->GetObject<MobilityModel>()->GetDistanceFrom(
                       nodes.Get(1)->GetObject<MobilityModel>());

    std::cout << "UAV_SUMMARY," << transportProt << "," << uavSpeed << "," << finalDist << ","
              << totalBytes << "," << avgThroughput << std::endl;

    g_metricStream.close();
    Simulator::Destroy();
    return 0;
}
