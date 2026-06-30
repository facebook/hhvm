/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <algorithm>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <initializer_list>
#include <iterator>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

#include <gtest/gtest.h>

#include <folly/testing/TestUtil.h>
#include <thrift/compiler/diagnostic.h>
#include <thrift/compiler/generate/formatter.h>
#include <thrift/compiler/parse/lexer.h>
#include <thrift/compiler/parse/parse_ast.h>
#include <thrift/compiler/parse/token.h>

using apache::thrift::compiler::diagnostic;
using apache::thrift::compiler::diagnostic_params;
using apache::thrift::compiler::diagnostic_results;
using apache::thrift::compiler::diagnostics_engine;
using apache::thrift::compiler::format_thrift_source;
using apache::thrift::compiler::lexer;
using apache::thrift::compiler::parse_ast;
using apache::thrift::compiler::source_manager;
using apache::thrift::compiler::source_range;
using apache::thrift::compiler::tok;
using apache::thrift::compiler::trivia_kind;

namespace {

std::string read_file(const std::filesystem::path& path) {
  std::ifstream input(path);
  return {
      std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>()};
}

void write_file(const std::filesystem::path& path, std::string_view text) {
  std::ofstream output(path);
  output << text;
}

std::vector<std::string> comment_texts(std::string_view source) {
  source_manager source_mgr;
  const auto source_view = source_mgr.add_virtual_file("test.thrift", source);
  diagnostic_results results;
  diagnostics_engine diags(source_mgr, results, diagnostic_params::strict());
  std::vector<std::string> comments;
  auto on_trivia = [&](trivia_kind kind, source_range range) {
    if (kind == trivia_kind::line_comment ||
        kind == trivia_kind::block_comment ||
        kind == trivia_kind::doc_comment) {
      comments.emplace_back(source_mgr.get_text_range(range));
    }
  };

  lexer lex(
      source_view,
      diags,
      [](std::string_view, source_range) {},
      std::move(on_trivia));
  while (lex.get_next_token().kind != tok::eof) {
  }
  if (results.has_error()) {
    throw std::runtime_error(results.diagnostics().front().str());
  }
  return comments;
}

void expect_comments_retained(std::string_view source) {
  const std::vector<std::string> before = comment_texts(source);
  const std::string formatted = format_thrift_source(source);
  EXPECT_EQ(comment_texts(formatted), before);
  EXPECT_EQ(format_thrift_source(formatted), formatted);
}

void format_file_in_place(const std::filesystem::path& path) {
  write_file(path, format_thrift_source(read_file(path)));
}

std::optional<std::filesystem::path> find_fixtures_root() {
  constexpr const char* env_var = "THRIFT_COMPILER_TEST_FIXTURES";
  const char* value = std::getenv(env_var);
  if (value == nullptr || std::string_view(value).empty()) {
    return std::nullopt;
  }

  std::filesystem::path path(value);
  if (!std::filesystem::is_directory(path)) {
    throw std::runtime_error(
        std::string(env_var) +
        " does not point to a directory: " + path.string());
  }
  return std::filesystem::canonical(path);
}

std::vector<std::filesystem::path> fixture_thrift_files(
    const std::filesystem::path& fixtures_root) {
  std::vector<std::filesystem::path> files;
  for (const auto& entry :
       std::filesystem::recursive_directory_iterator(fixtures_root)) {
    if (!entry.is_regular_file() || entry.path().extension() != ".thrift") {
      continue;
    }
    if (entry.path().parent_path().filename() != "src") {
      continue;
    }
    files.push_back(entry.path());
  }
  std::sort(files.begin(), files.end());
  return files;
}

void expect_format(std::string_view input, std::string_view expected) {
  EXPECT_EQ(format_thrift_source(input), expected);
}

struct formatter_case {
  std::string_view name;
  std::string_view input;
  std::string_view expected;
};

void expect_format_cases(std::initializer_list<formatter_case> cases) {
  for (const auto& test_case : cases) {
    SCOPED_TRACE(test_case.name);
    expect_format(test_case.input, test_case.expected);
  }
}

TEST(FormatterTest, formatsBasicHeadersAndDefinitions) {
  expect_format(
      R"(include    'foo'
  namespace    safe   bar
)",
      R"(include 'foo'
namespace safe bar
)");

  expect_format(
      R"(include 'foo'
            as
      bar
      ;
)",
      R"(include 'foo' as bar
)");

  expect_format(
      R"(const string
    foo = 'foo'

;
const
  i32 answer = 42
)",
      R"(const string foo = 'foo';
const i32 answer = 42;
)");

  expect_format(
      R"(typedef string
 ( hs.a1 = 1 , hs.a2 = 2) Name
 ( hs.a3 = 3)
typedef list<i64> Ids
)",
      R"(typedef string (hs.a1 = 1, hs.a2 = 2) Name (hs.a3 = 3)
typedef list<i64> Ids
)");
}

TEST(FormatterTest, formatsBasicTypesAndServices) {
  expect_format(
      R"(enum
  Color { RED,
BLUE = 2
}
)",
      R"(enum Color {
  RED,
  BLUE = 2,
}
)");

  expect_format(
      R"(struct Example {
1: optional string (hs.a1 = 1) name
2: required list<i32> values = [ 1 ,2,
  3]
}
)",
      R"(struct Example {
  1: optional string (hs.a1 = 1) name;
  2: required list<i32> values = [1, 2, 3];
}
)");

  expect_format(
      R"(service Foo {
oneway void bar () throws (1: string ex1)
  void    baz (1: string arg1)
    throws (1: string field1)
}
service Empty extends
  Base
   { }
)",
      R"(service Foo {
  oneway void bar() throws (1: string ex1);
  void baz(1: string arg1) throws (1: string field1);
}
service Empty extends Base {
}
)");
}

TEST(FormatterTest, formatsFunctionQualifiersAndContextualKeywords) {
  expect_format_cases({
      {
          "can format function options",
          R"THRIFT(
      service Foo {
        void  Fn1();
        oneway void  Fn2();
        readonly  void Fn3();
        idempotent void Fn4 ();
      }
    )THRIFT",
          R"THRIFT(service Foo {
  void Fn1();
  oneway void Fn2();
  readonly void Fn3();
  idempotent void Fn4();
}
)THRIFT",
      },
      {
          "can format all function kinds",
          R"THRIFT(
      service Foo {
        T  Fn1();
        oneway void  Fn2();
        stream<i32>  Fn3();
        Foo, sink<Bar, Baz>  Fn4();
        Interaction,  Struct  Fn5();
        Interaction, Struct , stream<i32 >  Fn6();
      }
    )THRIFT",
          R"THRIFT(service Foo {
  T Fn1();
  oneway void Fn2();
  stream<i32> Fn3();
  Foo, sink<Bar, Baz> Fn4();
  Interaction, Struct Fn5();
  Interaction, Struct, stream<i32> Fn6();
}
)THRIFT",
      },
      {
          "can format contextual keywords as idents",
          R"THRIFT(
      namespace safe transient
      struct permanent {
        1: i32 stateful;
      }
      exception server {}
      union client {}
      service oneway {}
    )THRIFT",
          R"THRIFT(namespace safe transient
struct permanent {
  1: i32 stateful;
}
exception server {}
union client {}
service oneway {
}
)THRIFT",
      },
      {
          "can format exception modifiers",
          R"THRIFT(
      safe
      transient
      server
      exception Foo {}

      permanent
      client
      exception Bar {}

      stateful
      exception Baz {}
    )THRIFT",
          R"THRIFT(safe transient server exception Foo {}

permanent client exception Bar {}

stateful exception Baz {}
)THRIFT",
      },
  });
}

TEST(FormatterTest, formatsEmptyDocuments) {
  expect_format("", "");
}

TEST(FormatterTest, formatsAnnotatedDefinitions) {
  expect_format_cases({
      {
          "can format enum definitions",
          R"THRIFT(
      enum
          foo {  NO_VALUE,
                 WITH_VALUE =    2,
          WITH_ANNOTATIONS (hs.a1 = 1)
                     }
                (hs.a2 = 2)
    )THRIFT",
          R"THRIFT(enum foo {
  NO_VALUE,
  WITH_VALUE = 2,
  WITH_ANNOTATIONS (hs.a1 = 1),
} (hs.a2 = 2)
)THRIFT",
      },
      {
          "can format struct definitions",
          R"THRIFT(
      struct foo {
1: optional string (hs.a1 = 1) bar1
2: required string (hs.a2 = 2) bar2;
3: string (hs.a3 = 3) bar1
            } (
              hs.a4 =4)
    )THRIFT",
          R"THRIFT(struct foo {
  1: optional string (hs.a1 = 1) bar1;
  2: required string (hs.a2 = 2) bar2;
  3: string (hs.a3 = 3) bar1;
} (hs.a4 = 4)
)THRIFT",
      },
      {
          "can format union definitions",
          R"THRIFT(
      union foo {
string bar;
            }
             ( hs.a1 =1)
    )THRIFT",
          R"THRIFT(union foo {
  string bar;
} (hs.a1 = 1)
)THRIFT",
      },
      {
          "can format interaction definitions",
          R"THRIFT(
      interaction X {
        string Y();
        string Z(
              1: int arg1,
              2: int arg2)
      }
    )THRIFT",
          R"THRIFT(interaction X {
  string Y();
  string Z(1: int arg1, 2: int arg2);
}
)THRIFT",
      },
      {
          "formats performs declarations inside services",
          R"THRIFT(
      service X {
          performs Y;
            // separator
        performs      Z;
      }
    )THRIFT",
          R"THRIFT(service X {
  performs Y;
  // separator
  performs Z;
}
)THRIFT",
      },
      {
          "can format exception definitions",
          R"THRIFT(
      exception foo {
        string bar;
            }
             ( hs.a1 =1)
    )THRIFT",
          R"THRIFT(exception foo {
  string bar;
} (hs.a1 = 1)
)THRIFT",
      },
  });
}

TEST(FormatterTest, formatsConstValuesAndLineBreaking) {
  expect_format_cases({
      {
          "can format constant values",
          R"THRIFT(
      const int _integer = 1;
        const hex  _hex = 0x51
            const double _double = 1.5
              const string _singleQuotedString = 'foo'
    const string _doubleQuotedString = "foo";
        const id _identifier = foo
        const list<int> _list = [
          1 ,2,
              3
        ]
    const map<string, int
              >     _map = { key1 : 1 , key2 :        2}
    )THRIFT",
          R"THRIFT(const int _integer = 1;
const hex _hex = 0x51;
const double _double = 1.5;
const string _singleQuotedString = 'foo';
const string _doubleQuotedString = "foo";
const id _identifier = foo;
const list<int> _list = [1, 2, 3];
const map<string, int> _map = {key1: 1, key2: 2};
)THRIFT",
      },
      {
          "can collapse groups correctly",
          R"THRIFT(
    service s {
      void short () throws ();
      void very_looooooooooonng_method_name (type very_loooooooooonong_parameter) throws (1: yet_extremely_loooooooooonong_type_name e);
    })THRIFT",
          R"THRIFT(service s {
  void short() throws ();
  void very_looooooooooonng_method_name(
    type very_loooooooooonong_parameter,
  ) throws (1: yet_extremely_loooooooooonong_type_name e);
}
)THRIFT",
      },
  });
}

TEST(FormatterTest, formatsCommentsAndSeparators) {
  expect_format_cases({
      {
          "can format comments",
          R"THRIFT(
// first line
/*
 * header
 */





// before
service x {
          // single line
  void foo();
}     // after block, single-line
service y {
  /*
   * multiple lines
   */
  void foo();
}       /* after block, multi-line
 */




/* at the end */     // after comment
)THRIFT",
          R"THRIFT(// first line
/*
 * header
 */

// before
service x {
  // single line
  void foo();
} // after block, single-line
service y {
  /*
   * multiple lines
   */
  void foo();
}
/* after block, multi-line
 */

/* at the end */ // after comment
)THRIFT",
      },
      {
          "can correct invalid separators",
          R"THRIFT(
      // should be converted to a semicolon:
      const string x = "value"
      service z {
      }       // should not add a separator
      // inner fields should have commas
      service a {
        void func1(1: string arg1 ; 2: string arg2);
      }
      // outer fields should have semicolons
      struct b {
        string field1,
      }

    )THRIFT",
          R"THRIFT(// should be converted to a semicolon:
const string x = "value";
service z {
} // should not add a separator
// inner fields should have commas
service a {
  void func1(1: string arg1, 2: string arg2);
}
// outer fields should have semicolons
struct b {
  string field1;
}
)THRIFT",
      },
      {
          "preserves end of line comments, even if separator is updated",
          R"THRIFT(
enum SourceEnumType {
  ONE = 1; // Generated and maintained by source one.
  TWO = 2; // Generated and maintained by source two.
  THREE = 3; // Generated and maintained by source three.
}
)THRIFT",
          R"THRIFT(enum SourceEnumType {
  ONE = 1, // Generated and maintained by source one.
  TWO = 2, // Generated and maintained by source two.
  THREE = 3, // Generated and maintained by source three.
}
)THRIFT",
      },
      {
          "should append last separator only if it was going to break",
          R"THRIFT(
        service MyService {
          void long (string long_paramter_1, string long_paramter_2,string long_paramter_3)
          void short (
            string s,
          )
        }
)THRIFT",
          R"THRIFT(service MyService {
  void long(
    string long_paramter_1,
    string long_paramter_2,
    string long_paramter_3,
  );
  void short(string s);
}
)THRIFT",
      },
      {
          "should preserve blank lines between members",
          R"THRIFT(
        package 'hi'

        include 'foo'

        include 'preserve/line/before/this'

        enum SourceEnumType {

          ONE = 1, // Remove line before

          TWO = 2, // Keep lines around

          THREE = 3, // Remove line after

      }
    )THRIFT",
          R"THRIFT(package 'hi'

include 'foo'

include 'preserve/line/before/this'

enum SourceEnumType {
  ONE = 1, // Remove line before

  TWO = 2, // Keep lines around

  THREE = 3, // Remove line after
}
)THRIFT",
      },
  });
}

TEST(FormatterTest, formatsContainerValues) {
  expect_format_cases({
      {
          "should break lines between map entries correctly",
          R"THRIFT(
        const short short_map = { x: 1, y:   2,}

        const map<StringType,i64>   REALLY_LONG_MAP = {
          Record_Number_1 : 167367433327742,
          Record_Number_2 : 60155386506,
          Record_Number_3 : 157260040977786,      Record_Number_4 : 102559116480387,
          Record_Number_5 : 394393211010,             Record_Number_6 : 147693381934060,
          Record_Number_7 : 24065685075,


          Record_Number_8 : 2374053504,
        }
    )THRIFT",
          R"THRIFT(const short short_map = {x: 1, y: 2};

const map<StringType, i64> REALLY_LONG_MAP = {
  Record_Number_1: 167367433327742,
  Record_Number_2: 60155386506,
  Record_Number_3: 157260040977786,
  Record_Number_4: 102559116480387,
  Record_Number_5: 394393211010,
  Record_Number_6: 147693381934060,
  Record_Number_7: 24065685075,
  Record_Number_8: 2374053504,
};
)THRIFT",
      },
      {
          "should break lines between list values correctly",
          R"THRIFT(
        const short short_list = [1,2,   3,]

        const list<StringType>   REALLY_LONG_LIST = [
          RECORD_NUMBER_1,    RECORD_NUMBER_2,
          RECORD_NUMBER_3,
          RECORD_NUMBER_4,
          RECORD_NUMBER_5,
          RECORD_NUMBER_6,  RECORD_NUMBER_7,RECORD_NUMBER_8
        ]
    )THRIFT",
          R"THRIFT(const short short_list = [1, 2, 3];

const list<StringType> REALLY_LONG_LIST = [
  RECORD_NUMBER_1,
  RECORD_NUMBER_2,
  RECORD_NUMBER_3,
  RECORD_NUMBER_4,
  RECORD_NUMBER_5,
  RECORD_NUMBER_6,
  RECORD_NUMBER_7,
  RECORD_NUMBER_8,
];
)THRIFT",
      },
  });
}

TEST(FormatterTest, formatsAnnotations) {
  expect_format_cases({
      {
          "can format annotations after enum declarations - multiple short ones",
          R"THRIFT(
    enum MyEnum {
      } (hs.a1 = 1,    hs.a2=2,    hs.a3    =    3)
    )THRIFT",
          R"THRIFT(enum MyEnum {
} (hs.a1 = 1, hs.a2 = 2, hs.a3 = 3)
)THRIFT",
      },
      {
          "can format annotations after enum declarations - one long attribute",
          R"THRIFT(
    enum MyEnum {
      } (hs.attribute1 = "really looooooooooooooooooooooooooooooooooooooooooooooooong value")
    )THRIFT",
          R"THRIFT(enum MyEnum {
} (
  hs.attribute1 = "really looooooooooooooooooooooooooooooooooooooooooooooooong value",
)
)THRIFT",
      },
      {
          "can format annotations after enum declarations - two long attributes",
          R"THRIFT(
      enum MyEnum {
        } (hs.attribute1 = "really looooooooooooooooooooooooooooooooooooooooooooooooong value",    hs.attribute2 = "really looooooooooooooooooooooooooooooooooooooooooooooooong value")
    )THRIFT",
          R"THRIFT(enum MyEnum {
} (
  hs.attribute1 = "really looooooooooooooooooooooooooooooooooooooooooooooooong value",
  hs.attribute2 = "really looooooooooooooooooooooooooooooooooooooooooooooooong value",
)
)THRIFT",
      },
  });
}

TEST(FormatterTest, formatsTypeReferencesAndInlineComments) {
  expect_format_cases({
      {
          "can parse generic types - single",
          R"THRIFT(
service TService {
    list<i32
    > getList(
    )
}
)THRIFT",
          R"THRIFT(service TService {
  list<i32> getList();
}
)THRIFT",
      },
      {
          "can parse generic types - multiple",
          R"THRIFT(
service TService {
    map<i32
    , i64> getMap(
    )
}
)THRIFT",
          R"THRIFT(service TService {
  map<i32, i64> getMap();
}
)THRIFT",
      },
      {
          "can handle multiline comments inline with code",
          R"THRIFT(
typedef i64 /* (cpp.type = "std::uint64_t") */ unsigned64
)THRIFT",
          R"THRIFT(typedef i64 /* (cpp.type = "std::uint64_t") */ unsigned64
)THRIFT",
      },
  });
}

TEST(FormatterTest, formatsAnnotationObjects) {
  expect_format_cases({
      {
          "can handle annotation objects - no body",
          R"THRIFT(
  service TService {
    @annotation
    string getList()
})THRIFT",
          R"THRIFT(service TService {
  @annotation
  string getList();
}
)THRIFT",
      },
      {
          "can handle annotation objects - empty body",
          R"THRIFT(
  service TService {
    @annotation {    }
    string getList()
})THRIFT",
          R"THRIFT(service TService {
  @annotation{}
  string getList();
}
)THRIFT",
      },
      {
          "can handle annotation objects - full body",
          R"THRIFT(
  service TService {
    @annotation{
      a =1, b = 2,
      }
    string getList()
})THRIFT",
          R"THRIFT(service TService {
  @annotation{a = 1, b = 2}
  string getList();
}
)THRIFT",
      },
      {
          "can handle annotation objects - break body",
          R"THRIFT(
  service TService {
    @annotation{
      reeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeally_long_a =1, reeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeally_long_b = 2,
      }
    string getList()
})THRIFT",
          R"THRIFT(service TService {
  @annotation{
    reeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeally_long_a = 1,
    reeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeally_long_b = 2,
  }
  string getList();
}
)THRIFT",
      },
      {
          "can handle annotation objects on headers",
          R"THRIFT(
      @annotation{
        a =1, b = 2,
        }
    package 'foo'

      @annotation{
        a =1, b = 2,
        }
    package;
)THRIFT",
          R"THRIFT(@annotation{a = 1, b = 2}
package 'foo'

@annotation{a = 1, b = 2}
package;
)THRIFT",
      },
  });
}

TEST(FormatterTest, formatsObjectValuesAndGenericConstValues) {
  expect_format_cases({
      {
          "can handle object values - no body",
          R"THRIFT(
    struct    Struct1
    {
      1: string id =    Struct2
    })THRIFT",
          R"THRIFT(struct Struct1 {
  1: string id = Struct2;
}
)THRIFT",
      },
      {
          "can handle object values - empty body",
          R"THRIFT(
    struct    Struct1
    {
      1: string id =    Struct2  {   }
    })THRIFT",
          R"THRIFT(struct Struct1 {
  1: string id = Struct2{};
}
)THRIFT",
      },
      {
          "can handle object values - full body",
          R"THRIFT(
    struct    Struct1
    {
      1: string id =    Struct2 { a= 1, b= 2}
    })THRIFT",
          R"THRIFT(struct Struct1 {
  1: string id = Struct2{a = 1, b = 2};
}
)THRIFT",
      },
      {
          "can handle single element generic types - single line",
          R"THRIFT(
    const    list  <
    string   > foo = [];)THRIFT",
          R"THRIFT(const list<string> foo = [];
)THRIFT",
      },
      {
          "can handle single element generic types - multi-line",
          R"THRIFT(
    const    list  <
    long__________________________________________________string   > foo = [];)THRIFT",
          R"THRIFT(const list<
  long__________________________________________________string
> foo = [];
)THRIFT",
      },
      {
          "can handle two elements generic types - single line",
          R"THRIFT(
    const    map  <  string,
     string   > foo = {};)THRIFT",
          R"THRIFT(const map<string, string> foo = {};
)THRIFT",
      },
      {
          "can handle two elements generic types - multi-line",
          R"THRIFT(
      const    map  <  long__________________________________________________string ,
     long__________________________________________________string
      > foo = {};)THRIFT",
          R"THRIFT(const map<
  long__________________________________________________string,
  long__________________________________________________string
> foo = {};
)THRIFT",
      },
  });
}

TEST(FormatterTest, preservesFinalComments) {
  expect_format_cases({
      {
          "maintains a blank line after the last comment - single line comment",
          R"THRIFT(
      const int x = 0;
      // no new line after this)THRIFT",
          R"THRIFT(const int x = 0;
// no new line after this
)THRIFT",
      },
      {
          "maintains a blank line after the last comment - multi line comment",
          R"THRIFT(
      const int x = 0;
      /* no new line after this */)THRIFT",
          R"THRIFT(const int x = 0;
/* no new line after this */
)THRIFT",
      },
  });
}

TEST(FormatterTest, formatsStructuredAnnotations) {
  expect_format_cases({
      {
          "format the new annotation syntax",
          R"THRIFT(
      @MyStructAnnotation{
             count = 123,
             nest = {
                    'name': "'nested'",
             }
      }
      struct MyStruct {
      } (
        hs.foo=    123
      )
      )THRIFT",
          R"THRIFT(@MyStructAnnotation{count = 123, nest = {'name': "'nested'"}}
struct MyStruct {} (hs.foo = 123)
)THRIFT",
      },
      {
          "format the structured annotation syntax",
          R"THRIFT(
      @A1@A2
      @A3
      struct MyStruct {
        @A1
        {a=4}@A2 @A3{}
        1: int foo;
      }
      )THRIFT",
          R"THRIFT(@A1
@A2
@A3
struct MyStruct {
  @A1{a = 4}
  @A2
  @A3{}
  1: int foo;
}
)THRIFT",
      },
      {
          "format multiple fields with the structured annotation syntax",
          R"THRIFT(
      @A1@A2
      @A3
      struct MyStruct {
        1: int a;


        @A1
        {a=4}@A2 @A3{}
        2: int b;


        @A1
        {a=4}@A2 @A3{}
        3: int c;

        4: int d;
      }
      )THRIFT",
          R"THRIFT(@A1
@A2
@A3
struct MyStruct {
  1: int a;

  @A1{a = 4}
  @A2
  @A3{}
  2: int b;

  @A1{a = 4}
  @A2
  @A3{}
  3: int c;

  4: int d;
}
)THRIFT",
      },
      {
          "format multiple structs and services with the structured annotation syntax",
          R"THRIFT(
      @A1
      struct MyStruct {
        1: int a;
      }

      @A1
      struct MyStruct2 {
        1: int a;
      }

      @A2@A3
      service MyService {
        void A();
      }


      @A2
      @A3
      service MyService2 {
        void A();
      }
      )THRIFT",
          R"THRIFT(@A1
struct MyStruct {
  1: int a;
}

@A1
struct MyStruct2 {
  1: int a;
}

@A2
@A3
service MyService {
  void A();
}

@A2
@A3
service MyService2 {
  void A();
}
)THRIFT",
      },
      {
          "format blank lines between enums and structs with and without the structured annotation syntax",
          R"THRIFT(
      enum MyEnum {
        a = 0,
        b = 1,
      }

      @A1
      struct MyStruct {
        1: int a;
      }

      struct MyStruct2 {
        1: int a;
      }


      @A1
      struct MyStruct3 {
        1: int a;
      }

      struct MyStruct4 {
        1: int a;
      }
      )THRIFT",
          R"THRIFT(enum MyEnum {
  a = 0,
  b = 1,
}

@A1
struct MyStruct {
  1: int a;
}

struct MyStruct2 {
  1: int a;
}

@A1
struct MyStruct3 {
  1: int a;
}

struct MyStruct4 {
  1: int a;
}
)THRIFT",
      },
      {
          "format structured annotations with embedded comments",
          R"THRIFT(
      @Annot {
        foo = [
          42, // comment
        ]
      }
      struct Empty {}
    )THRIFT",
          R"THRIFT(@Annot{
  foo = [
    42, // comment
  ],
}
struct Empty {}
)THRIFT",
      },
      {
          "can format empty {} with comments",
          R"THRIFT(
      @Annot {}
      struct Empty {}

      @Annot {
        // Hi
      }
      exception Foo {
        // Hi
      }

      @Annot { // Hi
      }
      struct Bar { // Hi
      }

      @Annot {} // Hi
      union Baz {} // Hi
    )THRIFT",
          R"THRIFT(@Annot{}
struct Empty {}

@Annot{// Hi
}
exception Foo {
// Hi
}

@Annot{} // Hi
struct Bar { // Hi
}

@Annot{} // Hi
union Baz {} // Hi
)THRIFT",
      },
  });
}

TEST(FormatterTest, preservesTrailingCommentsInStructuredDefinitions) {
  for (std::string ty : {"struct", "union", "exception"}) {
    SCOPED_TRACE(ty);
    expect_format(
        "\n       " + ty + R"( Foo {
        1: i32 a; // comment A
        2: double b; // comment B
        // Comment C
      }
      )",
        ty + R"( Foo {
  1: i32 a; // comment A
  2: double b; // comment B
  // Comment C
}
)");
    expect_format(
        "\n      " + ty + R"( Foo {
        1: i32 a; // comment A
        2: double b; // comment B
        // Comment C
        // Comment D
        // Comment E
      }
      )",
        ty + R"( Foo {
  1: i32 a; // comment A
  2: double b; // comment B
  // Comment C
  // Comment D
  // Comment E
}
)");
    expect_format(
        "\n      " + ty + R"( Foo {
        1: i32 a; // comment A
        2: double b; // comment B

        // Comment C
        // Comment D
      }
      )",
        ty + R"( Foo {
  1: i32 a; // comment A
  2: double b; // comment B
  // Comment C
  // Comment D
}
)");
    expect_format(
        "\n      " + ty + R"( Foo {
        1: i32 a; /* comment A */
        2: double b;
        /* comment B */
        /* Comment C */
      }
      )",
        ty + R"( Foo {
  1: i32 a;
  /* comment A */
  2: double b;
  /* comment B */
  /* Comment C */
}
)");
  }
}

TEST(FormatterTest, preservesCommentsAndRoundTrips) {
  const std::string source = R"(package "facebook.com/thrift/test"

// leading
struct S {
  1: i32 field; // trailing
}
)";

  folly::test::TemporaryDirectory dir;
  const auto thrift_file =
      std::filesystem::path(dir.path().string()) / "test.thrift";
  write_file(thrift_file, source);
  format_file_in_place(thrift_file);

  const std::string formatted = read_file(thrift_file);
  EXPECT_EQ(formatted, source);

  source_manager roundtrip_mgr;
  roundtrip_mgr.add_virtual_file("test.thrift", formatted);
  auto diags = diagnostics_engine(roundtrip_mgr, [](const diagnostic&) {});
  auto programs = parse_ast(roundtrip_mgr, diags, "test.thrift", {});
  EXPECT_FALSE(diags.has_errors());
  EXPECT_NE(programs, nullptr);

  format_file_in_place(thrift_file);
  const std::string formatted_again = read_file(thrift_file);
  EXPECT_EQ(formatted_again, formatted);
}

TEST(FormatterTest, retainsCommentsAroundConcreteTokens) {
  expect_comments_retained(R"(package "facebook.com/thrift/test"

// before const
const map<string, i32> Values = {
  // before key
  "one": 1, // after map entry
  // before map close
};

/* before struct */
@hs.Struct{name = "S"} // after leading annotation
struct S {
  // before field
  1: list<
    // before type arg
    string
  > names = [
    // before list value
    "a", // after list value
    // before list close
  ]; // after field
  // before struct close
}

service Service {
  // before function
  void f(
    // before param
    1: string arg, // after param
    // before params close
  ) throws (
    // before throws field
    1: string ex, // after throws field
    // before throws close
  ); // after function
  // before service close
}
)");
}

TEST(FormatterTest, preservesContentAfterTrailingLineComments) {
  expect_format(
      R"(struct Foo {
  1: Bar // This is a long bar comment
        bar;
  2: list<i32> // after generic type
        items;
  3: i32 count // after field name
        = // after equal
        1;
}
)",
      R"(struct Foo {
  1: Bar // This is a long bar comment
    bar;
  2: list<i32> // after generic type
    items;
  3: i32 count // after field name
    = // after equal
    1;
}
)");

  expect_format(
      R"(include "foo" // after include path
  as Foo
namespace cpp // after namespace language
  foo.bar
typedef list<i64> // after typedef type
  Ids
const i32 // after const type
  answer = // after const equal
  42
service Foo // after service name
  extends Bar {
  i32 // after return type
    get()
}
)",
      R"(include "foo" // after include path
  as Foo
namespace cpp // after namespace language
  foo.bar
typedef list<i64> // after typedef type
  Ids
const i32 // after const type
  answer = // after const equal
  42;
service Foo // after service name
  extends Bar {
  i32 // after return type
    get();
}
)");
}

TEST(FormatterTest, preservesKeywordTrailingLineComments) {
  expect_comments_retained(R"(package // package
"facebook.com/thrift/test"

include // include
"foo.thrift" // path
as // as
foo

namespace // namespace
cpp2 // language
foo

const // const
i32 answer = 42;

typedef // typedef
string Name

enum // enum
Color {
  RED = 1,
}

struct // struct
S {
  1: optional // optional
  string field;
}

safe // safe
exception // exception
E {
}

service // service
Service extends // extends
Base {
  performs // performs
  Interaction;
  oneway // oneway
  void f() throws // throws
  ();
}
)");
}

TEST(FormatterTest, preservesThrowsFieldLeadingComments) {
  expect_format(
      R"(service Foo {
  void lineComment(
    1: Request request,
  ) throws (
    // Exceptions
    1: Error error,
  );
  void docComment(
    1: Request request,
  ) throws (
    /** Throws documentation. */
    1: Error error,
  );
}
)",
      R"(service Foo {
  void lineComment(1: Request request) throws (
    // Exceptions
    1: Error error,
  );
  void docComment(1: Request request) throws (
    /** Throws documentation. */
    1: Error error,
  );
}
)");
}

TEST(FormatterTest, preservesTypedefTypeArgumentSeparatorComments) {
  expect_format(
      R"(typedef map<
  i32, // productType
  Stats
> StatsMap

typedef map<i64/* TreeIdx */ , InputLayout> InputBySourceTree
)",
      R"(typedef map<
  i32, // productType
  Stats
> StatsMap

typedef map<i64 /* TreeIdx */, InputLayout> InputBySourceTree
)");
}

TEST(FormatterTest, preservesCommentsBeforeServiceExtends) {
  expect_format(
      R"(service Foo /* Hello there! */ extends Bar {
}
)",
      R"(service Foo /* Hello there! */ extends Bar {
}
)");
}

TEST(FormatterTest, preservesBlockCommentsBeforeFieldSeparators) {
  expect_format(
      R"(struct Foo {
  1: i32 field /* trailing block */;
}
)",
      R"(struct Foo {
  1: i32 field /* trailing block */;
}
)");

  expect_format(
      R"(struct Foo {
  1: i32 field
    /* previous block */
    /* trailing block */;
}
)",
      R"(struct Foo {
  1: i32 field /* previous block */
  /* trailing block */;
}
)");
}

TEST(FormatterTest, preservesCommentsBeforeAnnotationAndObjectClose) {
  expect_format(
      R"(@foo.Annotation{
  name = "value",
//   sourceDimensions = [], please refer to https://fburl.com/ods3-source-dimensions for more details and a list of available options.
}
struct Foo {
  1: string bar;
}

const AssetXidTypeEntry ASSET_XID_TYPE_COMMENT_TRAP_ENTRY =
  AssetXidTypeEntry{
    xid_type = "COMMENT_TRAP",
// oncall_shortname = "old_oncall",
  };
)",
      R"(@foo.Annotation{
  name = "value",
//   sourceDimensions = [], please refer to https://fburl.com/ods3-source-dimensions for more details and a list of available options.
}
struct Foo {
  1: string bar;
}

const AssetXidTypeEntry ASSET_XID_TYPE_COMMENT_TRAP_ENTRY = AssetXidTypeEntry{
  xid_type = "COMMENT_TRAP",
// oncall_shortname = "old_oncall",
};
)");
}

TEST(FormatterTest, preservesValueSeparatorTrailingComments) {
  expect_format(
      R"(const currency.CurrencyAmount creditLimit = currency.CurrencyAmount{
  currency = "USD",
  amount = 1000000, // $10,000.00
};

const list<RestoreState> FailureRestoreStates = [
  4,  // RestoreState.FAILED,
  5,  // RestoreState.TIMEOUT,
];
)",
      R"(const currency.CurrencyAmount creditLimit = currency.CurrencyAmount{
  currency = "USD",
  amount = 1000000, // $10,000.00
};

const list<RestoreState> FailureRestoreStates = [
  4, // RestoreState.FAILED,
  5, // RestoreState.TIMEOUT,
];
)");
}

TEST(FormatterTest, preservesNestedObjectSeparatorTrailingComments) {
  expect_comments_retained(
      R"(const Request request = Request{
  steps = [
    Step{
      creditLineState = CreditLineState{
        creditLimit = currency.CurrencyAmount{
          currency = "USD",
          amount = 1000000, // $10,000.00
        },
      },
      },
  ],
};
)");
}

TEST(FormatterTest, preservesSourceWithMissingIncludeAndUnresolvedType) {
  const std::string source = R"(package "facebook.com/thrift/test"

include "missing.thrift"

struct S {
  1: missing.Missing field;
}
)";

  folly::test::TemporaryDirectory dir;
  const auto thrift_file =
      std::filesystem::path(dir.path().string()) / "test.thrift";
  write_file(thrift_file, source);
  format_file_in_place(thrift_file);

  const std::string formatted = read_file(thrift_file);
  EXPECT_EQ(formatted, source);
}

TEST(FormatterTest, rejectsParserInvalidSource) {
  EXPECT_THROW(
      format_thrift_source(R"(struct S { 1: list<i32 field; })"),
      std::runtime_error);
}

TEST(FormatterTest, fixtureFilesRoundTrip) {
  const auto fixtures_root = find_fixtures_root();
  if (!fixtures_root) {
    GTEST_SKIP() << "THRIFT_COMPILER_TEST_FIXTURES is not set";
  }

  const auto fixture_files = fixture_thrift_files(*fixtures_root);
  ASSERT_FALSE(fixture_files.empty());

  for (const auto& fixture_file : fixture_files) {
    SCOPED_TRACE(fixture_file.string());

    folly::test::TemporaryDirectory dir;
    const auto scratch_file =
        std::filesystem::path(dir.path().string()) / fixture_file.filename();
    write_file(scratch_file, read_file(fixture_file));
    const std::vector<std::string> source_comments =
        comment_texts(read_file(scratch_file));
    format_file_in_place(scratch_file);

    const std::string formatted = read_file(scratch_file);
    EXPECT_EQ(comment_texts(formatted), source_comments);
    EXPECT_EQ(format_thrift_source(formatted), formatted);
  }
}

} // namespace
