/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
*/

#ifndef incl_HPHP_PIDCONTOLLER_H_
#define incl_HPHP_PIDCONTOLLER_H_

#include <cmath>
#include <limits>

namespace HPHP {
/**
 * Like anything useful, we benefit from using a pid controller in certain
 * applications.  This is the 10 millionth implementation of a naive one.
 */

struct PIDController {
  PIDController() = default;
  PIDController(double dt, double max, double min,
                double kP, double kI, double kD)
      : dt(dt)
      , max(max)
      , min(min)
      , kP(kP)
      , kI(kI)
      , kD(kD) {}

  // This returns the new value for the controlled variable, and takes
  // `curValue`, the process variable, and the `setpoint`.
  double determineOutput(double setpoint, double curValue) {
    double error = setpoint - curValue;

    double proportionalTerm = kP * error;

    double integralDelta = error * dt;
    m_integral += integralDelta;

    double integralTerm = kI * m_integral;
    // This stops integral windup issues in cases the controlled variable is
    // temporarily not influenced by the contoller output.
    //
    // You may need to widen the max an min values slightly to have proper pid
    // behavior near those areas.
    if (integralTerm > max) {
      m_integral = max / kI;
      integralTerm = max;
    }
    if (integralTerm < min) {
      m_integral = min / kI;
      integralTerm = min;
    }

    // The derivative is based on values rather than error to not overrespond
    // during setpoint changes.
    double derivativeTerm = 0;
    if (m_initialized) {
      derivativeTerm = (m_lastValue - curValue) / dt * kD;
    } else {
      m_initialized = true;
    }

    m_lastValue = curValue;

    auto const output = proportionalTerm + integralTerm + derivativeTerm;
    if (output > max) return max;
    if (output < min) return min;
    return output;
  }

  double dt;
  double max;
  double min;
  double kP;
  double kI;
  double kD;

private:
  double m_integral{0};
  double m_lastValue{std::numeric_limits<double>::max()};
  bool m_initialized{false};
};


}

#endif // incl_HPHP_PIDCONTOLLER_H_
