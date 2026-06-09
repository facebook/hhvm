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

#include "hphp/runtime/vm/prologue-id.h"
#include "hphp/runtime/vm/jit/tc-recycle.h"

namespace HPHP {

struct Func;

struct FuncCleanup {

  struct Metadata {
    Metadata() = default;
    Metadata(Metadata&&) = default;
    Metadata& operator=(Metadata&&) = default;

    std::vector<SrcKey> srcKeys;
    std::vector<SrcKey> profDataSks;
    std::vector<PrologueID> profDataPrologueIDs;
    jit::tc::RecycleInfo recycleInfo;
  };

  static std::unique_ptr<FuncCleanup> get(FuncId funcId);
  static std::unique_ptr<FuncCleanup> get(const Func* func);

  Metadata& metadata();
  void cleanup();

  static void addSrcDBKey(const SrcKey sk);
  static void addProfDataSks(const SrcKey sk);
  static void addProfDataPrologueID(const PrologueID prologueID);

  explicit FuncCleanup(const Func* func, std::unique_lock<std::mutex> lock): m_func(func), m_lock(std::move(lock)) {}

 private:
  const Func* m_func;
  std::unique_lock<std::mutex> m_lock;
};

}
