<?hh


<<__EntryPoint>>
function main_rtit_prefixpart_outofrange() {
$rait = new RecursiveArrayIterator(varray[0,1,varray[2,3]]);
$rtit = new RecursiveTreeIterator($rait);

try {
  $rtit->setPrefixPart(6, "");
} catch (OutOfRangeException $e) {
  echo $e->getMessage(), PHP_EOL;
}
}
