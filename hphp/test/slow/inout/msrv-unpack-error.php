<?hh

function foo(inout $x, $y) {
  echo "whoops\n";
}

function main($y, $z) {
  var_dump($y(5, ...$z));
}


<<__EntryPoint>>
function main_msrv_unpack_error() {
main('foo$0$inout', array(12));
}
