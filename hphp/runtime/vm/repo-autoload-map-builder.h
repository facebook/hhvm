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

#include "hphp/runtime/base/repo-autoload-map.h"
#include "hphp/runtime/base/string-data.h"

#include "hphp/util/blob-encoder.h"
#include "hphp/util/hash-map.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////

struct UnitEmitter;
struct PreClassEmitter;
struct FuncEmitter;
struct TypeAlias;
struct Constant;
struct Module;

struct RepoAutoloadMapBuilder {

  template <typename Compare>
  using Map = folly_concurrent_hash_map_simd<
    const StringData*,
    int64_t,
    string_data_hash,
    Compare
  >;

  using TypeNameMap = Map<string_data_tsame>;
  using FuncNameMap = Map<string_data_fsame>;
  using CaseSensitiveMap = Map<string_data_same>;

  friend struct FuncEmitter;

  void addUnit(const UnitEmitter& ue);

  const TypeNameMap& getTypes() const;
  const FuncNameMap& getFuncs() const;
  const CaseSensitiveMap& getConstants() const;
  const TypeNameMap& getTypeAliases() const;
  const CaseSensitiveMap& getModules() const;

private:
  TypeNameMap m_types;
  FuncNameMap m_funcs;
  TypeNameMap m_typeAliases;
  CaseSensitiveMap m_constants;
  CaseSensitiveMap m_modules;
};

//////////////////////////////////////////////////////////////////////

}
