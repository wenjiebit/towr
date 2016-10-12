/**
 @file    nlp_optimizer_node.cc
 @author  Alexander W. Winkler (winklera@ethz.ch)
 @date    Jul 21, 2016
 @brief   Declares the interface to the ROS node that initializes/calls the NLP optimizer.
 */

#ifndef USER_TASK_DEPENDS_XPP_OPT_INCLUDE_XPP_ROS_NLP_OPTIMIZER_NODE_H_
#define USER_TASK_DEPENDS_XPP_OPT_INCLUDE_XPP_ROS_NLP_OPTIMIZER_NODE_H_

#include <xpp/ros/optimizer_node_base.h>
#include <xpp/ros/optimization_visualizer.h>
#include <xpp/hyq/hyq_spliner.h>
#include <xpp/opt/nlp_facade.h>
#include <xpp_msgs/RequiredInfoNlp.h>        // receive

namespace xpp {
namespace ros {

class NlpOptimizerNode : public OptimizerNodeBase {
public:
  typedef xpp::opt::NlpFacade NlpFacade;
  typedef xpp::hyq::LegID LegID;
  typedef xpp_msgs::RequiredInfoNlp ReqInfoMsg;

  using WholeBodyMapper = xpp::hyq::HyqSpliner;
  using OptVisualizerPtr = std::shared_ptr<OptimizationVisualizer>;

public:
  NlpOptimizerNode ();
  virtual ~NlpOptimizerNode () {};

private:
  NlpFacade nlp_facade_;
  WholeBodyMapper whole_body_mapper_;

  virtual void OptimizeTrajectory() override final;
//  virtual void PublishOptimizedValues() const override final;
  virtual void PublishTrajectory() const override final;
  void UpdateCurrentState(const ReqInfoMsg& msg);

  double max_cpu_time_; ///< maximum allowable time to spend on solving the NLP
  int curr_swingleg_;
  xpp::hyq::MarginValues supp_polygon_margins_;

  ::ros::Subscriber current_info_sub_;
//  ::ros::Publisher opt_params_pub_;
//  ::ros::Publisher trajectory_pub_;
  ::ros::Publisher trajectory_pub_rviz_;
  ::ros::Publisher trajectory_pub_hyqjoints_;

  void CurrentInfoCallback(const ReqInfoMsg& msg);

  OptVisualizerPtr optimization_visualizer_;
};

} /* namespace ros */
} /* namespace xpp */

#endif /* USER_TASK_DEPENDS_XPP_OPT_INCLUDE_XPP_ROS_OPTIMIZER_NODE_H_ */
