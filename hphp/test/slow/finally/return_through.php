<?hh

function foo($a) :mixed{
  try {
    return 2;
  } finally {
    var_dump($a);
  }
  var_dump("lol");
}

<<__EntryPoint>>
function main_return_through() :mixed{
var_dump(foo(4));
}
