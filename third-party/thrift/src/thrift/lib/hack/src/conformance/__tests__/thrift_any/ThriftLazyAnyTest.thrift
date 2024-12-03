include "thrift/lib/thrift/any.thrift"
include "thrift/annotation/hack.thrift"

package "facebook.com/thrift/test"

@hack.Adapter{name = "\ThriftLazyAnyAdapter"}
typedef any.Any ThriftLazyAny

@hack.Adapter{name = "\ThriftLazyAnySimpleJsonAdapter"}
typedef any.Any ThriftLazyAnySimpleJson

enum ExampleEnum {
  ENUM_VALUE_0 = 0,
  ENUM_VALUE_1 = 1,
}

struct MainStruct {
  1: ThriftLazyAny field;
}

struct ExampleStruct {
  1: i32 num;
  3: list<string> vec;
}

struct DifferentStruct {
  1: i32 num;
  2: string whatever;
}

struct AnyTestHelper {
  1: any.Any field;
}

struct OptionalStruct {
  1: optional ThriftLazyAny optional_field;
}

struct MainStructSimpleJson {
  1: ThriftLazyAnySimpleJson field;
}

struct HashsetStruct {
  1: set<i32> hashSet;
}
