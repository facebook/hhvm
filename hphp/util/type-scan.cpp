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

#include "hphp/util/type-scan.h"

#include <cstdio>
#include <dlfcn.h>
#include <memory>
#include <vector>

#include <folly/Format.h>

#include "hphp/util/embedded-data.h"

namespace {

////////////////////////////////////////////////////////////////////////////////

// List of types which should be ignored (including any bases) by the generated
// scanners. Add as needed.
const std::unordered_set<std::string> ignored = {
  "pthread_cond_t",
  "std::condition_variable",
  "st_mysql_bind",
};

// List of templates which should not be used to store request heap allocated
// values (because scanner aware variants exist).
const std::unordered_set<std::string> forbidden_template = {
  "boost::container::flat_map",
  "boost::container::flat_multimap",
  "boost::container::flat_multiset",
  "boost::container::flat_set",
  "HPHP::FixedVector",
  "HPHP::TinyVector",
  "std::deque",
  "std::forward_list",
  "std::list",
  "std::map",
  "std::multimap",
  "std::multiset",
  "std::priority_queue",
  "std::queue",
  "std::set",
  "std::stack",
  "std::unique_ptr",
  "std::unordered_map",
  "std::unordered_multimap",
  "std::unordered_multiset",
  "std::unordered_set",
  "std::vector"
};

const std::unordered_set<std::string> forced_conservative = {
  "boost::variant",
  "folly::Optional",
  "std::optional",
  "std::shared_ptr",
  "std::function"
};

std::string stripTemplateArgs(std::string name) {
  std::size_t open_count = 0;
  std::size_t open_begin;
  for (std::size_t i = 0; i < name.size();) {
    if (name[i] == '<') {
      if (!open_count) open_begin = i;
      ++open_count;
      ++i;
    } else if (name[i] == '>' && open_count > 0) {
      if (!--open_count) {
        name = name.erase(open_begin, i - open_begin + 1);
        i = open_begin;
      } else {
        ++i;
      }
    } else {
      ++i;
    }
  }
  return name;
}

////////////////////////////////////////////////////////////////////////////////

}

namespace HPHP { namespace type_scan { namespace detail {

////////////////////////////////////////////////////////////////////////////////

void conservative_stub(type_scan::Scanner& scanner,
                       const void* ptr,
                       std::size_t size) {
  scanner.conservative(ptr, size);
}

void noptrs_stub(type_scan::Scanner& scanner, const void* ptr,
                 std::size_t size) {
}

// Initialize metadata table, used before init() is called. Since before this,
// the only type-indices that can be present are "kIndexUnknown" and
// "kIndexUnknownNoPtrs".
const Metadata stub_metadata_table[] = {
  {"(UNKNOWN)", conservative_stub},
  {"(UNKNOWN NO-PTRS)", noptrs_stub}
};
const Metadata* g_metadata_table = stub_metadata_table;
std::size_t g_metadata_table_size = 2;

bool isIgnoredType(const std::string& name) {
  return ignored.count(stripTemplateArgs(name));
}

bool isForbiddenTemplate(const std::string& name) {
  return forbidden_template.count(stripTemplateArgs(name));
}

bool isForcedConservativeTemplate(const std::string& name) {
  return forced_conservative.count(stripTemplateArgs(name));
}

////////////////////////////////////////////////////////////////////////////////

}

using namespace detail;

void init() {
#if defined(__clang__)
  // Clang is currently broken... It doesn't emit uncalled member functions in a
  // template class, even when using ATTRIBUTE_USED. This prevents the custom
  // scanners from being emitted (silently), which causes all sorts of
  // problems. Punt for now until we can figure out a fix. This means that we'll
  // continue to just conservative scan everything. See t10336705.
  return;
#elif defined(__linux__) || defined(__FreeBSD__)

  // Find the shared object embedded within a custom section.
  embedded_data data;
  if (!get_embedded_data("type_scanners", &data)) {
    // no embedded data was built; fall back to conservative scan.
    return;
  }

  // Link in the embedded object.
  char tmp_filename[] = "/tmp/hhvm_type_scanner_XXXXXX";
  auto const handle = dlopen_embedded_data(data, tmp_filename);
  if (!handle) {
    throw InitException{"Failed to dlopen embedded data"};
  }

  // Find the initialization function.
  auto scanner_init = reinterpret_cast<const Metadata*(*)(std::size_t&)>(
    dlsym(handle, kInitFuncName)
  );
  if (!scanner_init) {
    throw InitException{
      folly::sformat("dlsym() fails: {}", dlerror())
    };
  }

  // And call it. The return value is a pointer to the new metadata table, and
  // the size of the table will be updated by passing by ref.
  if (auto table = scanner_init(g_metadata_table_size)) {
    g_metadata_table = table;
  } else {
    throw InitException{"Failed to load scanner table"};
  }
#endif

  // Some other platform... do nothing and continue to conservative scan for
  // now.
}

////////////////////////////////////////////////////////////////////////////////

}}
