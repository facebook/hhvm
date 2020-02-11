<?hh
<<__EntryPoint>>
function main_entry(): void {
    //line 681 ...
    $array = varray[varray[7,8,9],1,2,3,varray[4,5,6]];
  $arrayIterator = new ArrayIterator($array);
  try {
  $test = new CachingIterator($arrayIterator, 0);
  $test = new CachingIterator($arrayIterator, 1);
  $test = new CachingIterator($arrayIterator, 2);
  $test = new CachingIterator($arrayIterator, 3); // this throws an exception
  } catch (InvalidArgumentException $e){
    print  $e->getMessage() . "\n";
  }


  echo "===DONE===\n";
}
