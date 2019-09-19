<?hh

// Reverse sorter
function cmp($value1, $value2) {
  if($value1 == $value2) {
    return 0;
  }
  else if($value1 < $value2) {
    return 1;
  }
  else
    return -1;
}
<<__EntryPoint>>
function main_entry(): void {
  /* Prototype  : int ArrayObject::uasort(callback cmp_function)
   * Description: proto int ArrayIterator::uasort(callback cmp_function)
   Sort the entries by values user defined function.
   * Source code: ext/spl/spl_array.c
   * Alias to functions:
   */

  echo "*** Testing ArrayObject::uasort() : basic functionality ***\n";
  $ao = new ArrayObject(array(2,3,1));

  $ao->uasort(fun('cmp'));
  var_dump($ao);
  echo "===DONE===\n";
}
