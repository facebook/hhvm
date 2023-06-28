<?hh

type Foo = KeyedContainer<string, mixed>;

function get(): Foo {
  return __hhvm_intrinsics\launder_value(vec[]);
}
function test() :mixed{
  var_dump(get());
}
<<__EntryPoint>>
function main_entry(): void {
  test();
}
