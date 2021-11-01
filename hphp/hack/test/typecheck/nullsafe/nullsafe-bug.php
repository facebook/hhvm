<?hh
interface IFoo {
  public function genFoo(): Awaitable<?IFoo>;
}
function foo<T>(Awaitable<T> $x): T {
  throw new Exception("foo");
}
function bar(/* TAny */ $foo): ?Awaitable<?IFoo> {
  return foo($foo->genFoo())?->genFoo();
}
