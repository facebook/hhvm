<?hh

class RecursiveArrayIteratorIterator extends RecursiveIteratorIterator
{

  function callGetChildren() {
    return new StdClass();
  }
}


<<__EntryPoint>>
function main_unexpected_value_exception() {
try {
  $arr = varray[0, varray[1]];
  foreach(new RecursiveArrayIteratorIterator(
          new RecursiveArrayIterator($arr)) as $k=>$v) {
  }
} catch(UnexpectedValueException $e) {
  echo "UnexpectedValueException caught", PHP_EOL;
}
}
