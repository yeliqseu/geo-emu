/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/mobility-module.h"
#include "ns3/config-store-module.h"

// sns3-satellite related headers
#include <ns3/satellite-helper.h>
#include <ns3/satellite-stats-helper-container.h>
#include <ns3/satellite-enums.h>
#include <ns3/tap-bridge-module.h>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("SatTapEmuExample");

int
main (int argc, char *argv[])
{
  Time simTime = Seconds (60);
  Time interPacketInterval = MilliSeconds (100);

  // Command line arguments
  CommandLine cmd;
  cmd.AddValue ("simTime", "Total duration of the simulation", simTime);
  cmd.AddValue ("interPacketInterval", "Inter packet interval", interPacketInterval);
  cmd.Parse (argc, argv);

  GlobalValue::Bind ("SimulatorImplementationType", StringValue ("ns3::RealtimeSimulatorImpl"));
  GlobalValue::Bind ("ChecksumEnabled", BooleanValue (true));


  ConfigStore inputConfig;
  inputConfig.ConfigureDefaults ();

  // parse again so you can override default values from the command line
  cmd.Parse(argc, argv);

  // tracing configuration
  Config::SetDefault ("ns3::SatHelper::ScenarioCreationTraceEnabled", BooleanValue (true));
  Config::SetDefault ("ns3::SatHelper::PacketTraceEnabled", BooleanValue (false));
  // folder to save traces
  Config::SetDefault ("ns3::SatEnvVariables::SimulationCampaignName", StringValue ("sat-tap-emu"));
  Config::SetDefault ("ns3::SatEnvVariables::EnableSimulationOutputOverwrite", BooleanValue (true));
  Config::SetDefault ("ns3::SatUserHelper::BackboneNetworkType", EnumValue(SatUserHelper::NETWORK_TYPE_CSMA));
  SatPhyRxCarrierConf::ErrorModel em (SatPhyRxCarrierConf::EM_AVI);
  //double errorRate (0.10);
  //Config::SetDefault ("ns3::SatUtHelper::FwdLinkConstantErrorRate", DoubleValue (errorRate));
  Config::SetDefault ("ns3::SatUtHelper::FwdLinkErrorModel", EnumValue (em));

  // Create SatCom scenario. Here SIMPLE scenario is used, which includes one UT and one GW
  SatHelper::PreDefinedScenario_t satScenario = SatHelper::SIMPLE;
  Ptr<SatHelper> satHelper = CreateObjectWithAttributes<SatHelper> 
                                ("BeamNetworkAddress", Ipv4AddressValue ("10.20.0.0"),
                                 "BeamNetworkMask", Ipv4MaskValue ("255.255.0.0"),
                                 "GwNetworkAddress", Ipv4AddressValue ("10.30.0.0"),
                                 "GwNetworkMask", Ipv4MaskValue ("255.255.0.0"),
                                 "UtNetworkAddress", Ipv4AddressValue ("10.10.0.0"),
                                 "UtNetworkMask", Ipv4MaskValue ("255.255.0.0") );
  satHelper->CreatePredefinedScenario (satScenario);

  
  TapBridgeHelper tapBridge;
  Ptr<Node> utUser = satHelper->GetUtUsers ().Get (0);
  tapBridge.SetAttribute ("Mode", StringValue ("UseBridge"));
  tapBridge.SetAttribute ("DeviceName", StringValue ("utuser-tap"));
  tapBridge.Install (utUser, utUser->GetDevice (1));
  
  Ptr<Node> gwUser = satHelper->GetGwUsers ().Get (0);
  tapBridge.SetAttribute ("Mode", StringValue ("UseBridge"));
  tapBridge.SetAttribute ("DeviceName", StringValue ("gwuser-tap"));
  tapBridge.Install (gwUser, gwUser->GetDevice (1));

  // Output default attributes
  Config::SetDefault ("ns3::ConfigStore::Filename", StringValue ("sat-tap-emuattributes.xml"));
  Config::SetDefault ("ns3::ConfigStore::Mode", StringValue ("Save"));
  ConfigStore outputConfig;
  outputConfig.ConfigureAttributes ();
  
  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();
  Ipv4GlobalRoutingHelper g;
  Ptr<OutputStreamWrapper> routingStream = Create<OutputStreamWrapper> ("dynamic-global-routing.routes", std::ios::out);
  g.PrintRoutingTableAllAt (Seconds (1), routingStream);
  //Simulator::Stop(simTime);
  Simulator::Run ();
  Simulator::Destroy ();
  return 0;
}
