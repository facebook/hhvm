<?hh

function foo(inout $x, $y) {
  echo "whoops\n";
}

function main($y, $z) {
  var_dump($y(5, ...$z));
}

main('foo$0$inout', array(12));
