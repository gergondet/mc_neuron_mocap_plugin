/*
 * Copyright 2021 CNRS-UM LIRMM, CNRS-AIST JRL
 */

#pragma once

#include <mc_control/GlobalPlugin.h>
#include "MoCap.h"
#include "ClientSocket.h"
#include "SocketException.h"
#include <iostream>

namespace mc_plugin
{

struct mocap_plugin : public mc_control::GlobalPlugin
{
  void init(mc_control::MCGlobalController & controller, const mc_rtc::Configuration & config) override;

  void reset(mc_control::MCGlobalController & controller) override;

  void before(mc_control::MCGlobalController &) override;

  void after(mc_control::MCGlobalController & controller) override;

  mc_control::GlobalPlugin::GlobalPluginConfiguration configuration() override;

  ~mocap_plugin() override;

private:
  std::shared_ptr<ros::NodeHandle> nh_;
  MoCap_Data mocap_;

  std::string ip_;
  int n_port_;

  void ROS_Spinner();
  sva::PTransformd get_pose(MoCap_Body_part part);
  sva::MotionVecd get_vel(MoCap_Body_part part);
  Eigen::Vector3d get_accel(MoCap_Body_part part);
};

} // namespace mc_plugin