<?hh


<<__EntryPoint>>
function main_rtit_current() {
$rait = new RecursiveArrayIterator(varray[0,1,varray[2,3,varray[4,5],6,7],8,9,varray[0,1]]);

$rtit = new RecursiveTreeIterator($rait);
$rtit_curr = new RecursiveTreeIterator($rait, RecursiveTreeIterator::BYPASS_CURRENT);

foreach($rtit as $key=>$val) {
  var_dump($rtit->current());
}

foreach($rtit_curr as $key=>$val) {
  var_dump($rtit_curr->current());
}
}
