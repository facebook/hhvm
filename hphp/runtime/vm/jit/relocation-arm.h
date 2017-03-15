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

#ifndef incl_HPHP_RUNTIME_VM_JIT_RELOCATION_ARM_H_
#define incl_HPHP_RUNTIME_VM_JIT_RELOCATION_ARM_H_

#include "hphp/runtime/vm/jit/relocation.h"

namespace HPHP { namespace jit { namespace arm {

void adjustForRelocation(RelocationInfo&) {
  not_implemented();
}
void adjustForRelocation(RelocationInfo& rel, TCA srcStart, TCA srcEnd) {
  not_implemented();
}
void adjustCodeForRelocation(RelocationInfo& rel, CGMeta& fixups) {
  not_implemented();
}
void adjustMetaDataForRelocation(RelocationInfo&, AsmInfo*, CGMeta&) {
  not_implemented();
}
void findFixups(TCA start, TCA end, CGMeta& fixups) {
  not_implemented();
}
size_t relocate(RelocationInfo&, CodeBlock&, TCA, TCA, CodeBlock&, CGMeta&,
                TCA*) {
  not_implemented();
  return 0;
}

}}}

#endif
