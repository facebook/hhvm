/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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
#ifndef incl_HHBBC_INTERP_STATE_H_
#define incl_HHBBC_INTERP_STATE_H_

#include <vector>
#include <string>

#include "folly/Optional.h"

#include "hphp/hhbbc/index.h"
#include "hphp/hhbbc/type-system.h"

namespace HPHP { namespace HHBBC {

//////////////////////////////////////////////////////////////////////

struct ClassAnalysis;

//////////////////////////////////////////////////////////////////////

/*
 * Information about a pre-live ActRec.  Part of state tracked in
 * State.
 */
struct ActRec {
  explicit ActRec(FPIKind kind, folly::Optional<res::Func> f = folly::none)
    : kind(kind)
    , func(f)
  {}

  FPIKind kind;
  folly::Optional<res::Func> func;
};

/*
 * A program state at a position in a php::Block.
 */
struct State {
  bool initialized = false;
  bool thisAvailable = false;
  std::vector<Type> locals;
  std::vector<Type> stack;
  std::vector<ActRec> fpiStack;
};

/*
 * States are EqualityComparible (provided they are in-states for the
 * same block).
 */
bool operator==(const ActRec&, const ActRec&);
bool operator!=(const ActRec&, const ActRec&);
bool operator==(const State&, const State&);
bool operator!=(const State&, const State&);

//////////////////////////////////////////////////////////////////////

/*
 * PropertiesInfo returns the PropState for private instance and static
 * properties.
 *
 * During analysis the ClassAnalysis* is available and the PropState is
 * retrieved from there. However during optimization the ClassAnalysis is
 * not available and the PropState has to be retrieved off the Class in
 * the Index. In that case cls is nullptr and the PropState fields are
 * populated.
 */
struct PropertiesInfo {
  PropertiesInfo(const Index&, Context, ClassAnalysis*);

  PropState& privateProperties();
  PropState& privateStatics();
  const PropState& privateProperties() const;
  const PropState& privateStatics() const;

private:
  ClassAnalysis* const m_cls;
  PropState m_privateProperties;
  PropState m_privateStatics;
};

//////////////////////////////////////////////////////////////////////

/*
 * State merging functions.
 *
 * These return true if the destination state changed.
 */
bool merge_into(PropState&, const PropState&);
bool merge_into(ActRec&, const ActRec&);
bool merge_into(State&, const State&);

//////////////////////////////////////////////////////////////////////

/*
 * Functions to show various aspects of interpreter state as strings.
 */
std::string show(const ActRec& a);
std::string property_state_string(const PropertiesInfo&);
std::string state_string(const php::Func&, const State&);

//////////////////////////////////////////////////////////////////////

}}

#endif
