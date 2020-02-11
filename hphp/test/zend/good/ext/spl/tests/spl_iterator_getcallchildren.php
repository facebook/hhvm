<?hh
<<__EntryPoint>>
function main_entry(): void {
    //line 681 ...
    $array = varray[varray[7,8,9],1,2,3,varray[4,5,6]];
  $recursiveArrayIterator = new RecursiveArrayIterator($array);
  $test = new RecursiveIteratorIterator($recursiveArrayIterator);

  var_dump($test->current());
  $test->next();
  var_dump($test->current());
  try {
    $output = $test->callGetChildren();
  } catch (InvalidArgumentException $ilae){
    $output = null;  
    print "invalid argument exception\n";
  }
  var_dump($output);


  echo "===DONE===\n";
}
