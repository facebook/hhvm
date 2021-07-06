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
#include "hphp/runtime/base/backtrace.h"

#include <folly/portability/GTest.h>
#include <folly/test/JsonTestUtil.h>

#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/base/type-array.h"
#include "hphp/runtime/base/type-string.h"

#include "hphp/util/struct-log.h"

namespace HPHP {

  const StaticString
    s_file("file"),
    s_line("line"),
    s_function("function"),
    s_class("class"),
    s_type("type"),
    s_arrow("->"),
    s_object("object"),
    s_double_colon("::");

//////////////////////////////////////////////////////////////////////
Array mockBacktrace() {
  return make_vec_array(
    make_dict_array(
      s_file, "filename",
      s_line, "42",
      s_function, "function_name",
      s_type, s_double_colon,
      s_class, "Class"
    )
  );
}

TEST(Backtrace, FullFunctionNames) {
  StructuredLogEntry sample;
  {
    auto bt = mockBacktrace();
    addBacktraceToStructLog(bt, sample);
  }
  auto const expectedDescription = "{\"vecs\":{\"php_lines\":[\"42\"],"
    "\"php_functions\":[\"Class::function_name\"],\"php_files\":[\"filename\"]}"
    ",\"sets\":{},\"ints\":{}}";
  FOLLY_EXPECT_JSON_EQ(show(sample), expectedDescription);
}

TEST(Backtrace, FunctionNameWithObject) {
  StructuredLogEntry sample;
  {
    auto bt = make_vec_array(
      make_dict_array(
        s_file, "filename",
        s_line, "42",
        s_function, "function_name",
        s_type, s_arrow,
        s_object, "this",
        s_class, "Class"
      )
    );
    addBacktraceToStructLog(bt, sample);
  }
  auto const expectedDescription = "{\"vecs\":{\"php_lines\":[\"42\"],"
    "\"php_functions\":[\"Class->function_name\"],\"php_files\":[\"filename\"]}"
    ",\"sets\":{},\"ints\":{}}";
  FOLLY_EXPECT_JSON_EQ(show(sample), expectedDescription);
}

TEST(Backtrace, LongFunctionNames) {
  StructuredLogEntry sample;
  {
    auto bt = make_vec_array(
      make_dict_array(
        s_file, "filenameeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee",
        s_line, "42---------------------------------------------------------------------------------",
        s_function, "function_name--------------------------------------------------------------------",
        s_type, s_arrow,
        s_object, "this-------------------------------------------------------------------------------",
        s_class, "Class-----------------------------------------------------------------------------------------"
      )
    );
    addBacktraceToStructLog(bt, sample);
  }
  auto const expectedDescription = "{\"vecs\":{\"php_lines\":[\"42-------------"
    "--------------------------------------------------------------------\"],"
    "\"php_functions\":[\"Class------------------------------------------------"
    "------------------------------------------>function_name------------------"
    "--------------------------------------------------\"],\"php_files\":[\"fil"
    "enameeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee"
    "eeeeeee\"]}"
    ",\"sets\":{},\"ints\":{}}";
  FOLLY_EXPECT_JSON_EQ(show(sample), expectedDescription);
}


//////////////////////////////////////////////////////////////////////

}
