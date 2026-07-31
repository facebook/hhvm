# Copyright (c) Facebook, Inc. and its affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

BUILT_SOURCES = \
  gen-cpp2/CarbonTest_types.cpp \
  gen-cpp2/CarbonTest_types_binary.cpp \
  gen-cpp2/CarbonTest_types_compact.cpp \
  gen-cpp2/CarbonTest_types_serialization.cpp \
  gen-cpp2/CarbonTest_types.h \
  gen-cpp2/CarbonTest_types_custom_protocol.h \
  gen-cpp2/CarbonTest_data.cpp \
  gen-cpp2/CarbonTest_data.h \
  gen-cpp2/CarbonTest_constants.cpp \
  gen-cpp2/CarbonTest_constants.h

gen-cpp2/CarbonTest_types.cpp: gen-cpp2/CarbonTest_types.h
gen-cpp2/CarbonTest_types_binary.cpp: gen-cpp2/CarbonTest_types.h
gen-cpp2/CarbonTest_types_compact.cpp: gen-cpp2/CarbonTest_types.h
gen-cpp2/CarbonTest_types_serialization.cpp: gen-cpp2/CarbonTest_types.h
gen-cpp2/CarbonTest_types.h: gen-cpp2/CarbonTest_types_custom_protocol.h
gen-cpp2/CarbonTest_constants.cpp: gen-cpp2/CarbonTest_constants.h
gen-cpp2/CarbonTest_constants.h: gen-cpp2/CarbonTest_data.cpp
gen-cpp2/CarbonTest_types_custom_protocol.h: gen-cpp2/CarbonTest_constants.cpp
gen-cpp2/CarbonTest_data.cpp: gen-cpp2/CarbonTest_data.h
gen-cpp2/CarbonTest_data.h: CarbonTest.thrift
	@FBTHRIFT@ -gen mstch_cpp2:stack_arguments,sync_methods_return_try,deprecated_terse_writes,include_prefix=mcrouter/lib/network/test/gen -I $(top_srcdir)/.. -I $(INSTALL_DIR)/include/ $(srcdir)/CarbonTest.thrift

# thrift1 emits more files than BUILT_SOURCES lists, so remove the whole tree.
clean-local:
	-rm -rf gen-cpp2/
