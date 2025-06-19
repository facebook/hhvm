package "meta.com/thrift/core/protocol/_tests_/strict_union_test"

namespace hack ""

union SerializerTestStrictUnion {
  1: i32 int_value;
  2: string str_value;
  3: list<string> list_of_strings;
  4: set<string> set_of_strings;
  5: map<i32, string> map_of_int_to_strings;
}
