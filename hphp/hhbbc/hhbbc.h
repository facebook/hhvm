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

#include <vector>
#include <memory>
#include <string>
#include <utility>

#include "hphp/hhbbc/options.h"

#include "hphp/runtime/vm/repo-global-data.h"
#include "hphp/runtime/vm/unit-emitter.h"

#include "hphp/util/coro.h"
#include "hphp/util/extern-worker.h"
#include "hphp/util/struct-log.h"

namespace HPHP {
namespace HHBBC {

/*
 * This is the public API to HHBBC.
 */

//////////////////////////////////////////////////////////////////////

// Config which affects HHBBC. Any extern-worker which invokes HHBBC
// needs to initialize these.
struct Config {
  Options o;
  RepoGlobalData gd;
  static Config get(RepoGlobalData);
  template <typename SerDe> void serde(SerDe& sd) {
    sd(o)(gd);
  }
};

//////////////////////////////////////////////////////////////////////

// The "input" to whole_program. Encapsulates key/value
// pairs. UnitEmitters must be turned into Key/Value pairs first (by
// calling make). Then the Key and an extern_worker::Ref of the Value
// must be stored in the WholeProgramInput. The implementation of Key,
// Value, and WholeProgramInput are hidden to avoid leaking HHBBC
// internals.
struct WholeProgramInput {
  struct Key {
    void serde(BlobEncoder&) const;
    void serde(BlobDecoder&);

    struct Impl;
    struct Deleter { void operator()(Impl*) const; };
    std::unique_ptr<Impl, Deleter> m_impl;
  };
  struct Value {
    void serde(BlobEncoder&) const;

    struct Impl;
    struct Deleter { void operator()(Impl*) const; };
    std::unique_ptr<Impl, Deleter> m_impl;
  };

  WholeProgramInput();

  void add(Key, extern_worker::Ref<Value>);

  using KeyValueVec = std::vector<std::pair<Key, Value>>;
  static KeyValueVec make(std::unique_ptr<UnitEmitter>);

  struct Impl;
  struct Deleter { void operator()(Impl*) const; };
  std::unique_ptr<Impl, Deleter> m_impl;
};

//////////////////////////////////////////////////////////////////////

/*
 * Perform whole-program optimization on a set of UnitEmitters.
 */

// Called when an UnitEmitter is ready to be written to the repo.
using EmitCallback = std::function<void(std::unique_ptr<UnitEmitter>)>;

// When HHBBC is done with the executor and client (given in
// whole_program), they will be passed into this callback. This
// enables the caller to destroy the client asynchronously (since it
// can be slow) while HHBBC still runs.
using DisposeCallback =
  std::function<void(std::unique_ptr<TicketExecutor>,
                     std::unique_ptr<extern_worker::Client>)>;

void whole_program(WholeProgramInput inputs,
                   Config config,
                   std::unique_ptr<TicketExecutor> executor,
                   std::unique_ptr<extern_worker::Client> client,
                   const EmitCallback& callback,
                   DisposeCallback dispose,
                   StructuredLogEntry* sample,
                   int num_threads = 0);

//////////////////////////////////////////////////////////////////////

/*
 * Main entry point when the program should behave like hhbbc.
 */
int main(int argc, char** argv);

//////////////////////////////////////////////////////////////////////

}}
