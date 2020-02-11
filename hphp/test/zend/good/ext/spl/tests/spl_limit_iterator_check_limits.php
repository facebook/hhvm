<?hh
<<__EntryPoint>>
function main_entry(): void {
    $array = varray[varray[7,8,9],1,2,3,varray[4,5,6]];
  $arrayIterator = new ArrayIterator($array);

  try {
    $limitIterator = new LimitIterator($arrayIterator, -1);
  } catch (OutOfRangeException $e){
    print $e->getMessage(). "\n";
  }


  try {
    $limitIterator = new LimitIterator($arrayIterator, 0, -2);
  } catch (OutOfRangeException $e){
    print $e->getMessage() . "\n";
  }

  try {
    $limitIterator = new LimitIterator($arrayIterator, 0, -1);
  } catch (OutOfRangeException $e){
    print $e->getMessage() . "\n";
  }



  echo "===DONE===\n";
}
