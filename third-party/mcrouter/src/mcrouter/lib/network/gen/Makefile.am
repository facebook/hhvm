# Copyright (c) Facebook, Inc. and its affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

BUILT_SOURCES = \
  gen-cpp2/Common_types.cpp \
  gen-cpp2/Common_types.h \
  gen-cpp2/Common_types.tcc \
  gen-cpp2/Common_data.cpp \
  gen-cpp2/Common_data.h \
  gen-cpp2/Memcache_types.cpp \
  gen-cpp2/Memcache_types.h \
  gen-cpp2/Memcache_types.tcc \
  gen-cpp2/Memcache_data.cpp \
  gen-cpp2/Memcache_data.h \
  gen-cpp2/MemcacheAsyncClient.cpp \
  gen-cpp2/MemcacheAsyncClient.h \
  gen-cpp2/MemcacheService_constants.cpp \
  gen-cpp2/MemcacheService_constants.h \
  gen-cpp2/MemcacheService_types.cpp \
  gen-cpp2/MemcacheService_types.h \
  gen-cpp2/MemcacheService_types.tcc \
  gen-cpp2/MemcacheService_data.cpp \
  gen-cpp2/MemcacheService_data.h

gen-cpp2/Common_types.cpp: gen-cpp2/Common_types.h
gen-cpp2/Common_types.h: gen-cpp2/Common_types.tcc
gen-cpp2/Common_types.tcc: gen-cpp2/Common_data.cpp
gen-cpp2/Common_data.cpp: gen-cpp2/Common_data.h
gen-cpp2/Common_data.h: Common.thrift
	@FBTHRIFT@ -gen mstch_cpp2:stack_arguments,sync_methods_return_try,deprecated_terse_writes,include_prefix=mcrouter/lib/network/gen -I $(top_srcdir)/.. -I $(INSTALL_DIR)/include/ Common.thrift

gen-cpp2/Memcache_types.cpp: gen-cpp2/Memcache_types.h
gen-cpp2/Memcache_types.h: gen-cpp2/Memcache_types.tcc
gen-cpp2/Memcache_types.tcc: gen-cpp2/Memcache_data.cpp
gen-cpp2/Memcache_data.cpp: gen-cpp2/Memcache_data.h
gen-cpp2/Memcache_data.h: Memcache.thrift
	@FBTHRIFT@ -gen mstch_cpp2:stack_arguments,sync_methods_return_try,deprecated_terse_writes,include_prefix=mcrouter/lib/network/gen -I $(top_srcdir)/.. -I $(INSTALL_DIR)/include/ Memcache.thrift

gen-cpp2/MemcacheAsyncClient.cpp: gen-cpp2/MemcacheAsyncClient.h
gen-cpp2/MemcacheAsyncClient.h: gen-cpp2/MemcacheService_constants.cpp
gen-cpp2/MemcacheService_constants.cpp: gen-cpp2/MemcacheService_constants.h
gen-cpp2/MemcacheService_constants.h: gen-cpp2/MemcacheService_types.cpp
gen-cpp2/MemcacheService_types.cpp: gen-cpp2/MemcacheService_types.h
gen-cpp2/MemcacheService_types.h: gen-cpp2/MemcacheService_types.tcc
gen-cpp2/MemcacheService_types.tcc: gen-cpp2/MemcacheService_data.cpp
gen-cpp2/MemcacheService_data.cpp: gen-cpp2/MemcacheService_data.h
gen-cpp2/MemcacheService_data.h: MemcacheService.thrift
	@FBTHRIFT@ -gen mstch_cpp2:stack_arguments,sync_methods_return_try,deprecated_terse_writes,include_prefix=mcrouter/lib/network/gen -I $(top_srcdir)/.. -I $(INSTALL_DIR)/include/ MemcacheService.thrift
