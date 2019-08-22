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

#include "hphp/runtime/vm/jit/call-target-profile.h"

#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/vm/func.h"
#include "hphp/runtime/vm/jit/prof-data-serialize.h"

#include "hphp/util/trace.h"

#include <cstring>
#include <sstream>

namespace HPHP { namespace jit {

TRACE_SET_MOD(irlower);

///////////////////////////////////////////////////////////////////////////////

const size_t CallTargetProfile::kMaxEntries;

void CallTargetProfile::init() {
  if (m_init) return;
  m_init = true;
  for (size_t i = 0; i < kMaxEntries; i++) {
    m_entries[i] = Entry{};
  }
}

void CallTargetProfile::report(const Func* func) {
  assertx(func);
  auto const funcId = func->getFuncId();
  FTRACE(5, "CallTargetProfile::report: funcId {} ({})\n", funcId,
         func->fullName());
  init();
  size_t i = 0;
  for (; i < kMaxEntries; i++) {
    auto& entry = m_entries[i];
    if (entry.funcId == InvalidFuncId) {
      entry.funcId = funcId;
      entry.count = 1;
      return;
    }
    if (entry.funcId == funcId) {
      entry.count++;
      return;
    }
  }
  m_untracked++;
}

void CallTargetProfile::reduce(CallTargetProfile& profile,
                               const CallTargetProfile& other) {
  Entry allEntries[kMaxEntries * 2];
  size_t nEntries = 0;

  if (profile.m_init) {
    // Copy into `allEntries' all the valid entries from `profile'.
    for (size_t i = 0; i < kMaxEntries; i++) {
      auto const& entry = profile.m_entries[i];
      if (entry.funcId == InvalidFuncId) break;
      allEntries[nEntries++] = entry;
    }
  }

  if (other.m_init) {
    // Handle the entries from `other', either by adding to matching entries
    // from `profile' or by adding new entries to `allEntries'.
    for (size_t o = 0; o < kMaxEntries; o++) {
      auto const& otherEntry = other.m_entries[o];
      if (otherEntry.funcId == InvalidFuncId) break;
      size_t p = 0;
      for (; p < kMaxEntries; p++) {
        auto& entry = allEntries[p];
        if (entry.funcId == otherEntry.funcId) {
          entry.count += otherEntry.count;
          break;
        }
      }
      if (p == kMaxEntries) {
        // Didn't find `otherEntry' among the entries from `profile', so add a
        // new entry.
        allEntries[nEntries++] = otherEntry;
      }
    }
  }

  profile.m_init |= other.m_init;

  if (!profile.m_init) return;

  // Sort the entries in `allEntries' and select the top kMaxEntries.  The rest
  // is put in m_untracked.
  std::sort(allEntries, allEntries + nEntries,
            [&] (const Entry& a, const Entry& b) {
              // Sort in decreasing order of `count' while keeping invalid
              // entries at the end.
              if (b.funcId == InvalidFuncId) return a.funcId != InvalidFuncId;
              if (a.funcId == InvalidFuncId) return false;
              return a.count > b.count;
            });
  auto const nEntriesToCopy = std::min(kMaxEntries, nEntries);
  memcpy(profile.m_entries, allEntries, nEntriesToCopy * sizeof(Entry));

  // Update `profile's untracked count.
  profile.m_untracked += other.m_untracked;
  for (size_t i = kMaxEntries; i < nEntries; i++) {
    auto const& entry = allEntries[i];
    if (entry.funcId == InvalidFuncId) break;
    profile.m_untracked += entry.count;
  }
}

const Func* CallTargetProfile::choose(double& probability) const {
  if (!m_init) {
    probability = 0;
    return nullptr;
  }

  assertx(m_entries[0].funcId != InvalidFuncId);

  FTRACE(3, "CallTargetProfile::choose(): {}\n", *this);

  size_t bestIdx = 0;
  uint64_t total = m_untracked + m_entries[0].count;

  for (size_t i = 1; i < kMaxEntries ; i++) {
    auto const& entry = m_entries[i];
    if (entry.funcId == InvalidFuncId) break;
    total += entry.count;
    if (entry.count > m_entries[bestIdx].count) {
      bestIdx = i;
    }
  }

  auto const& best = m_entries[bestIdx];
  auto bestFunc = Func::fromFuncId(best.funcId);
  probability = (double)best.count / total;
  FTRACE(2, "CallTargetProfile::choose(): best funcId {} ({}), "
         "probability = {:.2}\n",
         best.funcId, bestFunc->fullName(), probability);
  return bestFunc;
}

std::string CallTargetProfile::toString() const {
  if (!m_init) return std::string("uninitialized");
  std::ostringstream out;
  uint64_t total = m_untracked;
  for (auto const& entry : m_entries) {
    if (entry.funcId != InvalidFuncId) {
      total += entry.count;
    }
  }
  for (auto const& entry : m_entries) {
    if (entry.funcId != InvalidFuncId) {
      out << folly::format("FuncId {}: {} ({:.1f}%), ",
                           entry.funcId, entry.count,
                           total == 0 ? 0 : 100.0 * entry.count / total);
    }
  }
  out << folly::format("Untracked: {} ({:.1f}%)", m_untracked,
                       total == 0 ? 0 : 100.0 * m_untracked / total);
  return out.str();
}

folly::dynamic CallTargetProfile::toDynamic() const {
  using folly::dynamic;

  if (!m_init) return dynamic();

  uint64_t total = m_untracked;
  for (auto const& entry : m_entries) {
    if (entry.funcId != InvalidFuncId) {
      total += entry.count;
    }
  }

  dynamic entries = dynamic::array;
  for (auto const& entry : m_entries) {
    if (entry.funcId != InvalidFuncId) {
      auto percent = total == 0 ? 0 : 100.0 * entry.count / total;
      entries.push_back(dynamic::object("funcId", entry.funcId)
                                       ("count", entry.count)
                                       ("percent", percent));
    }
  }

  auto percent = total == 0 ? 0 : 100.0 * m_untracked / total;
  dynamic untracked = dynamic::object("count", m_untracked)
                                     ("percent", percent);

  return dynamic::object("entries", entries)
                        ("untracked", untracked)
                        ("total", total)
                        ("profileType", "CallTargetProfile");
}

void CallTargetProfile::serialize(ProfDataSerializer& ser) const {
  for (size_t i = 0; i < kMaxEntries; i++) {
    auto const funcId = m_entries[i].funcId;
    const Func* func = funcId == kInvalidId ? nullptr
                                            : Func::fromFuncId(funcId);
    write_func(ser, func);
    write_raw(ser, m_entries[i].count);
  }
  write_raw(ser, m_untracked);
  write_raw(ser, m_init);
}

void CallTargetProfile::deserialize(ProfDataDeserializer& ser) {
  for (size_t i = 0; i < kMaxEntries; i++) {
    auto const func = read_func(ser);
    m_entries[i].funcId = func != nullptr ? func->getFuncId() : kInvalidId;
    read_raw(ser, m_entries[i].count);
  }
  read_raw(ser, m_untracked);
  read_raw(ser, m_init);
}

///////////////////////////////////////////////////////////////////////////////

}}
