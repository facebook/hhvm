<?hh

function foo($a) {
  try {
    return 2;
  } finally {
    var_dump($a);
  }
  var_dump("lol");
}

<<__EntryPoint>>
function main_return_through() {
var_dump(foo(4));
}
