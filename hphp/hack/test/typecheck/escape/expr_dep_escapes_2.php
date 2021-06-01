<?hh // strict

interface Foo<+T> {}

interface I {
  abstract const type T as Foo<this>;
  public function get(): this::T;
}

function getI(): I {
  while(true){};
}

function test(): int {
  $f = (() ==> getI()->get());
  $f();
  // the return type of $f should
  // be Foo<I>
  return $f();
}
