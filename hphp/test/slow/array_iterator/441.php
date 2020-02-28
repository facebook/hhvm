<?hh

function foo() {
  var_dump(__FUNCTION__);

 ArrayIterator441::$arr[] = 'bar';
}
function bar() {
 var_dump(__FUNCTION__);
 }

<<__EntryPoint>>
function array_iterator_441() {
  $arr = ArrayIterator441::$arr;
  reset(inout $arr);
  while (true) {
    $arr = ArrayIterator441::$arr;
    $func = each(inout $arr);
    if (!$func) {
      break;
    }
    ArrayIterator441::$arr = $arr;
   $f = $func[1];
   $f();
  }
}

abstract final class ArrayIterator441 {
  public static $arr = varray['bar', 'bar', 'bar', 'bar', 'bar', 'bar', 'foo'];
}
