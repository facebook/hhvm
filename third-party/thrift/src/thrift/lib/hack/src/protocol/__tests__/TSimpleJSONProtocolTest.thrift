package "meta.com/t_simple_json_protocol_test"

namespace php TSimpleJSONProtocolTest

struct StringVal {
  1: string s;
}

struct NumVals {
  1: i32 i;
  2: float f;
  3: map<i32, i32> m;
}

struct BinaryVal {
  1: binary s;
}
