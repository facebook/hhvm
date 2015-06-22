/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
   | Copyright (c) 1997-2010 The PHP Group                                |
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

#include "hphp/runtime/vm/resumable.h"
#include "hphp/runtime/ext/ext_generator.h"
#include "hphp/runtime/ext/asio/ext_async-generator.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

Resumable* Resumable::FromObj(ObjectData* obj) {
  auto const cls = obj->getVMClass();
  size_t objectOff = (cls == c_Generator::classof())
    ? GeneratorData::objectOff()
    : (cls == c_AsyncGenerator::classof())
    ?  AsyncGeneratorData::objectOff()
    :/*async function*/ 0;
  return reinterpret_cast<Resumable*>(
    reinterpret_cast<char*>(obj) - objectOff) - 1;
}

const Resumable* Resumable::FromObj(const ObjectData* obj) {
  auto const cls = obj->getVMClass();
  size_t objectOff = (cls == c_Generator::classof())
    ? GeneratorData::objectOff()
    : (cls == c_AsyncGenerator::classof())
    ?  AsyncGeneratorData::objectOff()
    :/*async function*/ 0;
  return reinterpret_cast<const Resumable*>(
    reinterpret_cast<const char*>(obj) - objectOff) - 1;
}

///////////////////////////////////////////////////////////////////////////////
}
