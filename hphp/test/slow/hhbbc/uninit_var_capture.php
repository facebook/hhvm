<?hh

function test() {
  $foo = () ==> $x;
  $x = 1;
  $foo();
}

test();
