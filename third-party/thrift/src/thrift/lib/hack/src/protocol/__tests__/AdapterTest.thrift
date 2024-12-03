package "meta.com/thrift/adapter_test"

namespace hack AdapterTest
include "thrift/annotation/hack.thrift"

@hack.Adapter{name = '\AdapterTestReverseList'}
typedef list<i32> ReversedList

@hack.Adapter{name = '\AdapterTestJsonToShape'}
typedef string StringWithAdapterTestJsonToShape

@hack.Adapter{name = '\AdapterTestStructToShape'}
typedef Bar BarWithAdapter

@hack.Adapter{name = '\AdapterTestIntToString'}
typedef i32 i32WithAdapter

struct Foo {
  1: i32WithAdapter intField;
  @hack.Adapter{name = '\AdapterTestIntToString'}
  2: optional i32 oIntField;
  3: list<StringWithAdapterTestJsonToShape> listField;
  4: optional list<StringWithAdapterTestJsonToShape> oListField;
  5: BarWithAdapter structField;
  6: optional BarWithAdapter oStructField;
  7: ReversedList reversedListField;
  8: map<string, BarWithAdapter> mapField;
}

struct FooWithoutAdapters {
  1: i32 intField;
  2: optional i32 oIntField;
  3: list<string> listField;
  4: optional list<string> oListField;
  5: Bar structField;
  6: optional Bar oStructField;
  7: list<i32> reversedListField;
  8: map<string, Bar> mapField;
}

struct Bar {
  1: i32 field;
}

service Service {
  ReversedList func(1: i32WithAdapter arg1, 2: Foo arg2);
}
