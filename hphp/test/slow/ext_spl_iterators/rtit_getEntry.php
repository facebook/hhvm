<?hh


<<__EntryPoint>>
function main_rtit_get_entry() {
$rait = new RecursiveArrayIterator(varray[0,1,varray[2,3,varray[4,5]]]);

$rtit = new RecursiveTreeIterator($rait);

foreach($rtit as $key=>$val) {
  var_dump($rtit->getEntry());
}
}
