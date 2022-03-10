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

#include <cstdio>
#include <cstring>
#include "hphp/runtime/vm/opcodes.h"

using namespace HPHP;

const char* fix_self(const char* name) {
  // We could avoid this by renaming Self to SelfCls
  return (strcmp(name, "Self") == 0) ? "Self_" : name;
}

#define IMM_TY_BLA      "ImmType::BLA"
#define IMM_TY_SLA      "ImmType::SLA"
#define IMM_TY_IVA      "ImmType::IVA"
#define IMM_TY_I64A     "ImmType::I64A"
#define IMM_TY_LA       "ImmType::LA"
#define IMM_TY_NLA      "ImmType::NLA"
#define IMM_TY_ILA      "ImmType::ILA"
#define IMM_TY_IA       "ImmType::IA"
#define IMM_TY_DA       "ImmType::DA"
#define IMM_TY_SA       "ImmType::SA"
#define IMM_TY_RATA     "ImmType::RATA"
#define IMM_TY_AA       "ImmType::AA"
#define IMM_TY_BA       "ImmType::BA"
#define IMM_TY_OA(type) "ImmType::OA(\"" #type "\")"
#define IMM_TY_VSA      "ImmType::VSA"
#define IMM_TY_KA       "ImmType::KA"
#define IMM_TY_LAR      "ImmType::LAR"
#define IMM_TY_ITA      "ImmType::ITA"
#define IMM_TY_FCA      "ImmType::FCA"

#define IMM_NAME_BLA(n)     "targets"
#define IMM_NAME_SLA(n)     "targets"
#define IMM_NAME_IVA(n)     "arg" #n
#define IMM_NAME_I64A(n)    "arg" #n
#define IMM_NAME_LA(n)      "loc" #n
#define IMM_NAME_NLA(n)     "nloc" #n
#define IMM_NAME_ILA(n)     "loc" #n
#define IMM_NAME_IA(n)      "iter" #n
#define IMM_NAME_DA(n)      "dbl" #n
#define IMM_NAME_SA(n)      "str" #n
#define IMM_NAME_RATA(n)    "rat"
#define IMM_NAME_AA(n)      "arr" #n
#define IMM_NAME_BA(n)      "target" #n
#define IMM_NAME_OA_IMPL(n) "subop" #n
#define IMM_NAME_OA(type)   IMM_NAME_OA_IMPL
#define IMM_NAME_VSA(n)     "keys"
#define IMM_NAME_KA(n)      "mkey"
#define IMM_NAME_LAR(n)     "locrange"
#define IMM_NAME_ITA(n)     "ita"
#define IMM_NAME_FCA(n)     "fca"

#define IMM_MEM(which, n)          printf("(\"%s\", %s), ", IMM_NAME_##which(n), IMM_TY_##which);
#define IMM_MEM_NA
#define IMM_MEM_ONE(x)                IMM_MEM(x, 1);
#define IMM_MEM_TWO(x, y)             IMM_MEM(x, 1); IMM_MEM(y, 2);
#define IMM_MEM_THREE(x, y, z)        IMM_MEM(x, 1); IMM_MEM(y, 2); \
                                      IMM_MEM(z, 3);
#define IMM_MEM_FOUR(x, y, z, l)      IMM_MEM(x, 1); IMM_MEM(y, 2); \
                                      IMM_MEM(z, 3); IMM_MEM(l, 4);
#define IMM_MEM_FIVE(x, y, z, l, m)   IMM_MEM(x, 1); IMM_MEM(y, 2); \
                                      IMM_MEM(z, 3); IMM_MEM(l, 4); \
                                      IMM_MEM(m, 5);
#define IMM_MEM_SIX(x, y, z, l, m, n) IMM_MEM(x, 1); IMM_MEM(y, 2); \
                                      IMM_MEM(z, 3); IMM_MEM(l, 4); \
                                      IMM_MEM(m, 5); IMM_MEM(n, 6);

#define IN_NOV "Inputs::NOV"
#define IN_ONE(a) "Inputs::Fixed([FlavorDesc::" #a "].into())"
#define IN_TWO(a,b) "Inputs::Fixed([FlavorDesc::" #a ", FlavorDesc::" #b "].into())"
#define IN_THREE(a,b,c) "Inputs::Fixed([FlavorDesc::" #a ", FlavorDesc::" #b ", FlavorDesc::" #c "].into())"
#define IN_SMANY "Inputs::SMany"
#define IN_CMANY "Inputs::CMany"
#define IN_CUMANY "Inputs::CUMany"
#define IN_MFINAL "Inputs::MFinal"
#define IN_C_MFINAL(n) "Inputs::CMFinal(" #n ")"
#define IN_FCALL(nin,nobj) "Inputs::FCall{inp: " #nin ", obj: " #nobj "}"

#define OUT_NOV "Outputs::NOV"
#define OUT_ONE(a) "Outputs::Fixed([FlavorDesc::" #a "].into())"
#define OUT_TWO(a,b) "Outputs::Fixed([FlavorDesc::" #a ", FlavorDesc::" #b "].into())"
#define OUT_THREE(a,b,c) "Outputs::Fixed([FlavorDesc::" #a ", " #b  ", FlavorDesc::" #c "].into())"
#define OUT_FCALL "Outputs::FCall"

#define FLAGS_NF "InstrFlags::NF"
#define FLAGS_CF "InstrFlags::CF"
#define FLAGS_TF "InstrFlags::TF"
#define FLAGS_CF_TF "InstrFlags::CF | InstrFlags::TF"

#define O(name, imms, in, out, flags)                    \
  printf("            OpcodeData{");                     \
  printf("name: \"%s\", ", fix_self(#name));             \
                                                         \
  printf("immediates: vec![");                           \
  IMM_MEM_##imms                                         \
  printf("], ");                                         \
                                                         \
  printf("inputs: %s, ", IN_##in);                       \
  printf("outputs: %s, ", OUT_##out);                    \
  printf("flags: %s, ", FLAGS_##flags);                  \
  printf("},\n");

// hhbc-gen/gen-rust.cpp:
//
// Print a Rust description of the opcodes with one OpcodeData for each opcode.
// The resulting definition contains all the information present in the C++
// OPCODES macro, with the intention that downstream Rust code of various kinds
// can be generated by proc-macros from the single OPCODES-derived Rust data.
//
// See test_util::test_opcodes() for sample output.
//
int main(int, char**) {
  printf("// Copyright (c) Facebook, Inc. and its affiliates.\n");
  printf("//\n");
  printf("// This source code is licensed under the MIT license found in the\n");
  printf("// LICENSE file in the \"hack\" directory of this source tree.\n");
  printf("// %sgenerated by gen-rust.cpp\n", "@");
  printf("// buck2 run :gen-rust > opcodes.rs\n");
  printf("\n");
  printf("pub use crate::{InstrFlags, Inputs, OpcodeData, Outputs, FlavorDesc, ImmType};\n");
  printf("use once_cell::sync::OnceCell;\n");
  printf("\n");
  printf("pub fn opcode_data() -> &'static [OpcodeData] {\n");
  printf("    static INSTANCE: OnceCell<Box<[OpcodeData]>> = OnceCell::new();\n");
  printf("    INSTANCE.get_or_init(|| {\n");
  printf("        vec![\n");
  OPCODES
  printf("        ].into()\n");
  printf("    })\n");
  printf("}\n");
  return 0;
}
