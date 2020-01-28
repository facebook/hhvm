<?hh


<<__EntryPoint>>
function main_rtit_get_prefix() {
$rait = new RecursiveArrayIterator(varray[0,1,varray[2,3,varray[4,5],6,7],8,9,varray[0,1]]);

$rtit = new RecursiveTreeIterator($rait);

foreach($rtit as $key=>$val) {
  var_dump($rtit->getPrefix());
}
}
