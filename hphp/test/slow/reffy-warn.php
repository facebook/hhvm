<?hh

class Foo {
  function alpha($one, inout $two, $three, inout $four) {}
}

class Bar extends Foo {
  function alpha(inout $one, $two, $three, inout $four) {}
}

<<__EntryPoint>>
function main(): void {
  echo "Done.\n";
}
