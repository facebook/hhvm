<?hh

<<__EntryPoint>>
function main_rtit_set_postfix() {
$rait = new RecursiveArrayIterator(varray[]);
$rtit = new RecursiveTreeIterator($rait);

var_dump($rtit->getPostfix());
var_dump($rtit->setPostfix('xx'));
var_dump($rtit->getPostfix());
}
