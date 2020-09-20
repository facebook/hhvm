<?hh

type t = @darray<int, @string>;
type u = varray<@int>;

function f<T>(): void {}

function g(@int $_): @string {
  f<@float>();
  return "hello";
}

abstract class C {
  <<TestAttr>> protected @float $x;
  const type T = @int;
  public function __construct(<<TestAttr>> public @int $y) {}
  public async function f(): Awaitable<@int> {
    return 42;
  }
}
