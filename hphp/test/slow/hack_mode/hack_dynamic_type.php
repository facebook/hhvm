<?hh

function foo<T as dynamic>(dynamic $x, @dynamic $z) : ?dynamic {
  return "anything";
}
type Something = dynamic;
newtype Foo = dynamic;

class C {
  static vec<dynamic> $z;
  const type T = dynamic;

}
function test() : void {
  foo(5, "string");
  foo(new C(), 5);
}


<<__EntryPoint>>
function main_hack_dynamic_type() {
test();
var_dump("done");
}
