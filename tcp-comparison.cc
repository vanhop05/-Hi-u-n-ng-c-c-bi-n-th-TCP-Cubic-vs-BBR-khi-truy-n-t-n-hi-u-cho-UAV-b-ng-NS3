#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/error-model.h"
#include "ns3/tcp-header.h"

#include <fstream>
#include <iostream>
#include <string>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("TcpComparison");

std::ofstream g_cwndStream;
std::ofstream g_throughputStream;
uint64_t g_lastTotalRx = 0;

static void
CwndChange(uint32_t oldCwnd, uint32_t newCwnd)
{
    g_cwndStream << Simulator::Now().GetSeconds() << "," << (newCwnd / 1448.0) << std::endl;
}

static void
TraceCwnd()
{
    Config::ConnectWithoutContext("/NodeList/0/$ns3::TcpL4Protocol/SocketList/0/CongestionWindow",
                                  MakeCallback(&CwndChange));
}

static void
TraceThroughput(Ptr<PacketSink> sink, double interval)
{
    double now = Simulator::Now().GetSeconds();
    uint64_t currentTotalRx = sink->GetTotalRx();
    double curThroughput = (currentTotalRx - g_lastTotalRx) * 8.0 / (interval * 1000000.0);
    g_lastTotalRx = currentTotalRx;

    g_throughputStream << now << "," << curThroughput << std::endl;

    Simulator::Schedule(Seconds(interval), &TraceThroughput, sink, interval);
}

int
main(int argc, char* argv[])
{
    std::string transportProt = "TcpCubic";
    std::string bandwidth = "10Mbps";
    std::string delay = "20ms";
    double errorRate = 0.01;
    double simTime = 20.0;
    std::string prefix = "results";

    CommandLine cmd(__FILE__);
    cmd.AddValue("transportProt", "TCP Protocol: TcpCubic or TcpBbr", transportProt);
    cmd.AddValue("bandwidth", "Bottleneck bandwidth", bandwidth);
    cmd.AddValue("delay", "Bottleneck delay", delay);
    cmd.AddValue("errorRate", "Packet loss rate", errorRate);
    cmd.AddValue("simTime", "Simulation time in seconds", simTime);
    cmd.AddValue("prefix", "Output files prefix", prefix);
    cmd.Parse(argc, argv);

    Config::SetDefault("ns3::TcpL4Protocol::SocketType", StringValue("ns3::" + transportProt));
    Config::SetDefault("ns3::TcpSocket::SegmentSize", UintegerValue(1448));

    NodeContainer nodes;
    nodes.Create(2);

    PointToPointHelper p2p;
    p2p.SetDeviceAttribute("DataRate", StringValue(bandwidth));
    p2p.SetChannelAttribute("Delay", StringValue(delay));

    NetDeviceContainer devices = p2p.Install(nodes);

    if (errorRate > 0.0)
    {
        Ptr<RateErrorModel> em = CreateObject<RateErrorModel>();
        em->SetAttribute("ErrorRate", DoubleValue(errorRate));
        em->SetAttribute("ErrorUnit", StringValue("ERROR_UNIT_PACKET"));
        devices.Get(1)->SetAttribute("ReceiveErrorModel", PointerValue(em));
    }

    InternetStackHelper internet;
    internet.Install(nodes);

    Ipv4AddressHelper ipv4;
    ipv4.SetBase("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer interfaces = ipv4.Assign(devices);

    uint16_t port = 8080;
    Address sinkAddress(InetSocketAddress(interfaces.GetAddress(1), port));
    PacketSinkHelper sinkHelper("ns3::TcpSocketFactory", sinkAddress);
    ApplicationContainer sinkApp = sinkHelper.Install(nodes.Get(1));
    sinkApp.Start(Seconds(0.0));
    sinkApp.Stop(Seconds(simTime));

    BulkSendHelper sourceHelper("ns3::TcpSocketFactory", sinkAddress);
    sourceHelper.SetAttribute("MaxBytes", UintegerValue(0));
    ApplicationContainer sourceApp = sourceHelper.Install(nodes.Get(0));
    sourceApp.Start(Seconds(1.0));
    sourceApp.Stop(Seconds(simTime));

    std::string cwndFile = prefix + "_" + transportProt + "_cwnd.csv";
    std::string throughputFile = prefix + "_" + transportProt + "_throughput.csv";
    g_cwndStream.open(cwndFile.c_str());
    g_cwndStream << "Time,Cwnd_Packets\n";
    g_throughputStream.open(throughputFile.c_str());
    g_throughputStream << "Time,Throughput_Mbps\n";

    g_lastTotalRx = 0;
    Simulator::Schedule(Seconds(1.00001), &TraceCwnd);
    Ptr<PacketSink> sink = DynamicCast<PacketSink>(sinkApp.Get(0));
    Simulator::Schedule(Seconds(1.1), &TraceThroughput, sink, 0.1);

    Simulator::Stop(Seconds(simTime + 0.1));
    Simulator::Run();

    uint64_t totalBytes = sink->GetTotalRx();
    double avgThroughput = (totalBytes * 8.0) / ((simTime - 1.0) * 1000000.0);

    std::cout << "SUMMARY," << transportProt << "," << bandwidth << "," << delay << ","
              << errorRate << "," << totalBytes << "," << avgThroughput << std::endl;

    g_cwndStream.close();
    g_throughputStream.close();
    Simulator::Destroy();
    return 0;
}
