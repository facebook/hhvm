<?hh

function less($a, $b) :mixed{
 return $a < $b;
 }

function main($a) :mixed{
  usort(inout $a, less<>);
  var_dump($a);
}


<<__EntryPoint>>
function main_usort() :mixed{
main(varray[1,2]);
}
