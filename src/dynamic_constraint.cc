/**
 @file    dynamic_constraint.cc
 @author  Alexander W. Winkler (winklera@ethz.ch)
 @date    Dec 5, 2016
 @brief   Brief description
 */

#include <xpp/opt/dynamic_constraint.h>

#include "../include/xpp/opt/base_motion.h"

namespace xpp {
namespace opt {

using Vector2d = Eigen::Vector2d;
using JacobianRow = Eigen::SparseVector<double, Eigen::RowMajor>;

DynamicConstraint::DynamicConstraint ()
{
  kHeight_ = 0.0;
  name_ = "Dynamic";
}

DynamicConstraint::~DynamicConstraint ()
{
  // TODO Auto-generated destructor stub
}

void
DynamicConstraint::Init (const BaseMotion& com_motion,
                         const CenterOfPressure& cop,
                         double T,
                         double dt)
{
  com_motion_ = com_motion.clone();
  kHeight_ = com_motion.GetZHeight();
  cop_ = cop;

  // zmp_ DRY with other constraints?
  double t = 0.0;
  dts_.clear();
  for (int i=0; i<floor(T/dt); ++i) {
    dts_.push_back(t);
    t += dt;
  }
}

void
DynamicConstraint::UpdateVariables (const OptimizationVariables* opt_var)
{
  VectorXd x_coeff   = opt_var->GetVariables(com_motion_->GetID());
  com_motion_->SetOptimizationParameters(x_coeff);

  VectorXd cop = opt_var->GetVariables(cop_.GetID());
  cop_.SetOptimizationParameters(cop);
}

DynamicConstraint::VectorXd
DynamicConstraint::EvaluateConstraint () const
{
  int m = dts_.size()*kDim2d;
  Eigen::VectorXd g(m);

  int k = 0;
  for (double t : dts_) {

    auto com = com_motion_->GetCom(t);
    model_.SetCurrent(com.p, com.v, kHeight_);

    // acceleration as predefined by physics
    Vector2d acc_physics = model_.GetDerivative(cop_.GetCop(t));
    g.middleRows<kDim2d>(kDim2d*k) = acc_physics - com.a;

    k++;
  }

  return g;
}

VecBound
DynamicConstraint::GetBounds () const
{
  std::vector<Bound> bounds;
  int m = EvaluateConstraint().rows();
  for (int i=0; i<m; ++i)
    bounds.push_back(kEqualityBound_);

  return bounds;
}

DynamicConstraint::Jacobian
DynamicConstraint::GetJacobianWrtCop () const
{
  int m = GetNumberOfConstraints();
  Jacobian jac(m, cop_.GetOptVarCount());

  int row=0;
  for (double t : dts_) {

    auto com = com_motion_->GetCom(t);
    model_.SetCurrent(com.p, com.v, kHeight_);

    for (auto dim : d2::AllDimensions)
      jac.row(row++) = model_.GetJacobianApproxWrtCop(dim)*cop_.GetJacobianWrtCop(t,dim);
  }

  return jac;
}

DynamicConstraint::Jacobian
DynamicConstraint::GetJacobianWrtCom () const
{
  int m = GetNumberOfConstraints();
  Jacobian jac(m, com_motion_->GetOptVarCount());

  int n=0;
  for (double t : dts_) {

    auto com = com_motion_->GetCom(t);
    model_.SetCurrent(com.p, com.v, kHeight_);
    Vector2d cop = cop_.GetCop(t);

    for (auto dim : {X, Y}) {
      Jacobian jac_acc     = com_motion_->GetJacobian(t,kAcc,dim);
      Jacobian jac_physics = model_.GetJacobianApproxWrtSplineCoeff(*com_motion_, t, dim, cop);
      jac.row(kDim2d*n + dim) = jac_physics - jac_acc;
    }

    n++;
  }

  return jac;
}

DynamicConstraint::Jacobian
DynamicConstraint::GetJacobianWithRespectTo (std::string var_set) const
{
  Jacobian jac; // empty matrix

  if (var_set == cop_.GetID())
    jac = GetJacobianWrtCop();

  if (var_set == com_motion_->GetID())
    jac = GetJacobianWrtCom();

  return jac;
}

} /* namespace opt */
} /* namespace xpp */
