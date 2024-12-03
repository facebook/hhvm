include "thrift/annotation/hack.thrift"
include "thrift/annotation/scope.thrift"

package "meta.com/thrift/field_wrapper_test"

namespace hack FieldWrapperTest

@scope.Transitive
@hack.FieldWrapper{name = "\\MyFieldWrapper"}
struct TestAnnotation {}

struct MyStruct {
  @hack.FieldWrapper{name = "\\MyFieldWrapper"}
  1: i64 wrapped_field;
  @TestAnnotation
  2: i64 annotated_field;
  @hack.Adapter{name = '\\AdapterTestIntToString'}
  3: i64 adapted_type;
  @hack.Adapter{name = '\\AdapterTestIntToString'}
  @TestAnnotation
  4: i64 adapted_and_wrapped_type;
}

union MyUnion {
  @TestAnnotation
  1: i64 union_annotated_field;
  @hack.Adapter{name = '\\AdapterTestIntToString'}
  3: i64 union_adapted_type;
}

exception MyException {
  1: i64 code;
  2: string message;
  @TestAnnotation
  3: string annotated_message;
}

service Service {
  i32 func(1: string arg1, 2: MyStruct arg2);
}
