#include "plugin.h"

#include <mc_control/GlobalPluginMacros.h>

namespace mc_plugin
{

mocap_plugin::~mocap_plugin() = default;

void mocap_plugin::init(mc_control::MCGlobalController & controller, const mc_rtc::Configuration & config)
{

  config("ip",ip_);
  config("port",n_port_);

  mc_rtc::log::info("[mocap plugin] Initialize ROS Bridge");
  nh_ = mc_rtc::ROSBridge::get_node_handle();
  std::thread ROS_Thread(&mocap_plugin::ROS_Spinner, this);
  ROS_Thread.detach();
  mocap_.node_sub(*nh_);

  controller.controller().datastore().make<Eigen::MatrixXd>("mocap_plugin::LeftHand_pose_seq");
  controller.controller().datastore().make<Eigen::MatrixXd>("mocap_plugin::RightHand_pose_seq");
  controller.controller().datastore().make<Eigen::MatrixXd>("mocap_plugin::LeftHand_acc_seq");
  controller.controller().datastore().make<Eigen::MatrixXd>("mocap_plugin::RightHand_acc_seq");

  controller.controller().datastore().make<bool>("mocap_plugin::online");

  controller.controller().datastore().make_call(
      "mocap_plugin::get_pose", [this](MoCap_Body_part part) -> sva::PTransformd { return mocap_.get_pose(part); });
  controller.controller().datastore().make_call(
      "mocap_plugin::get_velocity", [this](MoCap_Body_part part) -> sva::MotionVecd { return mocap_.get_vel(part); });
  controller.controller().datastore().make_call(
      "mocap_plugin::get_accel",
      [this](MoCap_Body_part part) -> Eigen::Vector3d { return mocap_.get_linear_acc(part); });
  controller.controller().datastore().make_call(
      "mocap_plugin::get_footstate", [this](MoCap_Body_part part) -> int { return mocap_.foot_contact(part); });

  


  try
    {

      ClientSocket client_socket_ = ClientSocket( ip_, n_port_ );

      std::string reply;

      try
      {
        // client_socket << "Test message.";
        client_socket_ >> reply;
      }
      catch ( SocketException& ) {}

      std::cout << "We received this response from the server:\n\"" << reply << "\"\n";;

    }
  catch ( SocketException& e )
    {
      std::cout << "Exception was caught:" << e.description() << "\n";
    }




  mc_rtc::log::info("mocap_plugin::init called with configuration:\n{}", config.dump(true, true));

}

void mocap_plugin::reset(mc_control::MCGlobalController & controller)
{
  mc_rtc::log::info("mocap_plugin::reset called");
}

void mocap_plugin::before(mc_control::MCGlobalController &)
{
  mc_rtc::log::info("mocap_plugin::before called");
}

void mocap_plugin::after(mc_control::MCGlobalController & controller)
{
  controller.controller().datastore().assign<Eigen::MatrixXd>("mocap_plugin::LeftHand_acc_seq",
                                                              mocap_.get_LeftHandAcc_seq().transpose());
  controller.controller().datastore().assign<Eigen::MatrixXd>("mocap_plugin::LeftHand_pose_seq",
                                                              mocap_.get_LeftHandPose_seq().transpose());

  controller.controller().datastore().assign<Eigen::MatrixXd>("mocap_plugin::RightHand_acc_seq",
                                                              mocap_.get_RightHandAcc_seq().transpose());
  controller.controller().datastore().assign<Eigen::MatrixXd>("mocap_plugin::RightHand_pose_seq",
                                                              mocap_.get_RightHandPose_seq().transpose());
  controller.controller().datastore().assign<bool>("mocap_plugin::online", mocap_.Datas_Online());

  mc_rtc::log::info(mocap_.get_pose(LeftHand).translation());
}

mc_control::GlobalPlugin::GlobalPluginConfiguration mocap_plugin::configuration()
{
  mc_control::GlobalPlugin::GlobalPluginConfiguration out;
  out.should_run_before = false;
  out.should_run_after = true;
  out.should_always_run = true;
  return out;
}

void mocap_plugin::ROS_Spinner()
{
  ros::Rate loop_rate(60);

  try
  {

    ClientSocket client_socket_ = ClientSocket( ip_, n_port_ );
    while(ros::ok)
    {
      std::string data;
      try
      {
        client_socket_ >> data;
      }
      catch ( SocketException& ) {}
      // std::cout << "data " << data << std::endl;
      mocap_.convert_data(data);
      ros::spinOnce();
      mocap_.tick(1 / 60);
      loop_rate.sleep();
    }
  }
  catch ( SocketException& e )
  {
    std::cout << "Exception was caught:" << e.description() << "\n";
  }

}

} // namespace mc_plugin

EXPORT_MC_RTC_PLUGIN("mocap_plugin", mc_plugin::mocap_plugin)