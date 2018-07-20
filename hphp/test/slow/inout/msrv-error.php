<?hh

function foo(inout $x) {
  echo "whoops\n";
}

function main($y) {
  var_dump($y(12));
}

main('foo$0$inout');
