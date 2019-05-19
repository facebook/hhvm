<?hh

trait T {
  function __construct(inout $x) {}
}

class C {
  use T;
}

<<__EntryPoint>> function main(): void {}
