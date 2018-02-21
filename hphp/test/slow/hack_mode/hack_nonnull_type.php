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

test();
var_dump("done");
