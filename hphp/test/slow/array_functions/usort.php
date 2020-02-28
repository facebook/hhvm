<?hh

function less($a, $b) {
 return $a < $b;
 }

function main($a) {
  usort(inout $a, fun('less'));
  var_dump($a);
}


<<__EntryPoint>>
function main_usort() {
main(varray[1,2]);
}
