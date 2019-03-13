<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

type Foo = KeyedContainer<string, mixed>;

function get(): Foo {
  return __hhvm_intrinsics\launder_value(vec[]);
}
function test() {
  var_dump(get());
}
test();
