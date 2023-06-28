<?hh

class Foo {
  function alpha($one, inout $two, $three, inout $four) :mixed{}
}

class Bar extends Foo {
  function alpha(inout $one, $two, $three, inout $four) :mixed{}
}

<<__EntryPoint>>
function main(): void {
  echo "Done.\n";
}
