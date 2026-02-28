<?hh

class Foo {}
class Bar {}

function main($x) :mixed{
  if (!is_array($x)) $x = vec[$x];
  echo is_array($x);
  echo "\n";
  echo (bool)$x;
  echo "\n";
}


<<__EntryPoint>>
function main_jmp_type_002() :mixed{
main(12);
main(vec[1,2,3]);
}
