<?hh

class Foo {}

function main(?Foo $x = null) :mixed{
  if (!$x) {
    echo (string)(is_object($x)) . "\n";
  } else {
    echo is_object($x) . "\n";
  }
  return 2;
}




<<__EntryPoint>>
function main_jmp_local_001() :mixed{
main();
}
