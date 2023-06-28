<?hh

function foo<T as dynamic>(dynamic $x, <<__Soft>> dynamic $z) : ?dynamic {
  return "anything";
}

type Something = dynamic;
newtype Foo = dynamic;

function bar(Something $x, Foo $y): void {}

class C {
  public static vec<dynamic> $z;
  const type T = dynamic;

}
function test() : void {
  foo(5, "string");
  foo(new C(), false);
  bar(5, "string");
  bar(new C(), false);
}


<<__EntryPoint>>
function main_hack_dynamic_type() :mixed{
test();
var_dump("done");
}
