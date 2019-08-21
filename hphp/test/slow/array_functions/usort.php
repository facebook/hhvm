<?hh

function less($a, $b) {
 return $a < $b;
 }

function main($a) {
  usort(&$a, fun('less'));
  var_dump($a);
}


<<__EntryPoint>>
function main_usort() {
main(array(1,2));
}
