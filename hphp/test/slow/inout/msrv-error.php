<?hh

function foo(inout $x) {
  echo "whoops\n";
}

function main($y) {
  var_dump($y(12));
}


<<__EntryPoint>>
function main_msrv_error() {
main('foo$0$inout');
}
