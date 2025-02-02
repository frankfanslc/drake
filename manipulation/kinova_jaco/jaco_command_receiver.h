#pragma once

#include <memory>
#include <string>
#include <vector>

#include "drake/common/drake_copyable.h"
#include "drake/common/drake_deprecated.h"
#include "drake/common/eigen_types.h"
#include "drake/lcmt_jaco_command.hpp"
#include "drake/manipulation/kinova_jaco/jaco_constants.h"
#include "drake/systems/framework/leaf_system.h"

namespace drake {
namespace manipulation {
namespace kinova_jaco {

/// Handles lcmt_jaco_command message from a LcmSubscriberSystem.
///
/// Note that this system does not actually subscribe to an LCM channel. To
/// receive the message, the input of this system should be connected to a
/// LcmSubscriberSystem::Make<drake::lcmt_jaco_command>().
///
/// It has one required input port, "lcmt_jaco_command".
///
/// This system has a single output port which contains the commanded position
/// and velocity for each joint.  Finger velocities will be translated from
/// the values used by the Kinova SDK to values appropriate for the finger
/// joints in the Jaco description (see jaco_constants.h).
///
/// @system
/// name: JacoCommandReceiver
/// input_ports:
/// - lcmt_jaco_command
/// - position_measured (optional)
/// output_ports:
/// - state
///
/// @par Output prior to receiving a valid lcmt_jaco_command message:
/// The "position" output initially feeds through from the "position_measured"
/// input port -- or if not connected, outputs zero.  When discrete update
/// events are enabled (e.g., during a simulation), the system latches the
/// "position_measured" input into state during the first event, and the
/// "position" output comes from the latched state, no longer fed through from
/// the "position" input.  Alternatively, the LatchInitialPosition() method is
/// available to achieve the same effect without using events.
///
/// @endsystem
class JacoCommandReceiver : public systems::LeafSystem<double> {
 public:
  DRAKE_NO_COPY_NO_MOVE_NO_ASSIGN(JacoCommandReceiver)

  JacoCommandReceiver(int num_joints = kJacoDefaultArmNumJoints,
                      int num_fingers = kJacoDefaultArmNumFingers);

  /// (Advanced) Copies the current "position_measured" input (or zero if not
  /// connected) into Context state, and changes the behavior of the "position"
  /// output to produce the latched state if no message has been received yet.
  /// The latching already happens automatically during the first discrete
  /// update event (e.g., when using a Simulator); this method exists for use
  /// when not already using a Simulator or other special cases.
  void LatchInitialPosition(systems::Context<double>* context) const;

  /// @name Named accessors for this System's input and output ports.
  //@{
  const systems::InputPort<double>& get_message_input_port() const {
    return *message_input_;
  }
  const systems::InputPort<double>& get_position_measured_input_port() const {
      return *position_measured_input_;
  }
  //@}

  DRAKE_DEPRECATED("2022-06-01",
     "To provide position commands prior to receiving the first message, "
     "connect the position_measured ports instead of setting this "
     "parameter")
  void set_initial_position(
      systems::Context<double>* context,
      const Eigen::Ref<const Eigen::VectorXd>& q) const;

  DRAKE_DEPRECATED("2022-06-01", "Use get_message_input_port() instead.")
  const systems::InputPort<double>& get_input_port() const {
    return get_message_input_port();
  }

 private:
  Eigen::VectorXd input_state(const systems::Context<double>&) const;
  void CalcInput(const systems::Context<double>&, lcmt_jaco_command*) const;

  void DoCalcNextUpdateTime(
      const systems::Context<double>&,
      systems::CompositeEventCollection<double>*, double*) const final;
  void CalcPositionMeasuredOrZero(
      const systems::Context<double>&, systems::BasicVector<double>*) const;

  // Copies the current "position measured" input (or zero if not connected)
  // into the @p result.
  void LatchInitialPosition(
      const systems::Context<double>&,
      systems::DiscreteValues<double>* result) const;

  const int num_joints_;
  const int num_fingers_;
  const systems::InputPort<double>* message_input_{};
  const systems::InputPort<double>* position_measured_input_{};
  const systems::CacheEntry* position_measured_or_zero_{};
  systems::DiscreteStateIndex latched_position_measured_is_set_;
  systems::DiscreteStateIndex latched_position_measured_;
  const systems::CacheEntry* groomed_input_{};
};

}  // namespace kinova_jaco
}  // namespace manipulation
}  // namespace drake
