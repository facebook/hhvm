<?hh

class Foo {}
class Bar {}

function main($x) :mixed{
  if (is_int($x)) {
    echo is_int($x);
    echo "\n";
  } else {
    if (is_array($x)) {
      echo is_array($x);
      echo (bool)$x;
      echo "\n";
    }
  }
}


<<__EntryPoint>>
function main_jmp_type_001() :mixed{
main(12);
main(varray[1,2,3]);
}
