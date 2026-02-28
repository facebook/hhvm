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

#pragma once

#include "hphp/runtime/base/types.h"

namespace HPHP {

struct StringData;
struct Unit;
struct UnitEmitter;

struct LazyStringData {

  LazyStringData(): m_id(kInvalidId) {}
  LazyStringData(Id id): m_id(id) {}

  LazyStringData& operator=(Id id) {
    m_id = id;
    return *this;
  }

  const Id id() const { return m_id; }

  const StringData* get(const Unit* unit) const;
  const StringData* get(const UnitEmitter& ue) const;

  template <typename SerDe>
  void serde(SerDe& sd) {
    sd(m_id);
  }

 private:
  Id m_id;
};

}
