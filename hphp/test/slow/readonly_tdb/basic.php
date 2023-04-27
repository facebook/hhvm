<?hh

namespace ROTDB\Basic;

class Foo {
  public int $x = 0;
}

function ret_ro(): readonly Foo {
  return new Foo();
}

<<__EntryPoint>>
function main(): void {
  $_res = ret_ro();
}
