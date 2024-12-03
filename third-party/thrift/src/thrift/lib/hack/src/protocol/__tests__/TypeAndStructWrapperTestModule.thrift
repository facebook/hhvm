include "TypeAndStructWrapperTest.thrift"
include "thrift/annotation/hack.thrift"

package "meta.com/thrift/wrapper_test"

namespace hack "WrapperTest"

struct ModuleMyStruct {
  1: i64 int_field;
}

union MyUnion {
  @TypeAndStructWrapperTest.AnnotationStruct
  1: i64 union_annotated_field;
  @hack.Adapter{name = "\AdapterTestIntToString"}
  3: i64 union_adapted_type;
}

exception MyException {
  1: i64 code;
  2: string message;
  @TypeAndStructWrapperTest.AnnotationStruct
  3: string annotated_message;
}

service Service {
  i32 func(1: string arg1, 2: TypeAndStructWrapperTest.TestMyStruct arg2);
}
