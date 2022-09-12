namespace java com.facebook.mojo.example

include "Example2.thrift"

struct Example1 {
  1: optional i32 bar;
  2: optional string baz;
}

struct TestsTransitiveThriftFileInclusion {
  1: optional Example2.Example2 fieldWithTypeFromTransitiveThriftFileInclusion;
}
