<?hh

function foo<T as nonnull>(nonnull $x, @nonnull $z) : ?nonnull {
  return "anything";
}
type Something = nonnull;
newtype Foo = nonnull;

class C {
  static vec<nonnull> $z;
  const type T = nonnull;

}
function test() : void {
  foo(5, "string");
  foo(new C(), 5);
}


<<__EntryPoint>>
function main_hack_nonnull_type() {
test();
var_dump("done");
}
