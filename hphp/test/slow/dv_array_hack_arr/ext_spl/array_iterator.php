<?hh

<<__EntryPoint>>
function main() {
  $inputs = vec[
    __hhvm_intrinsics\dummy_cast_to_kindofarray(vec[]),
    varray[1, 2, 3],
    darray['foo' => 1, 'bar' => 3, 'baz' => 5],
  ];

  foreach ($inputs as $arr) {
    $i = new ArrayIterator($arr);
    while ($i->valid()) {
      var_dump($i->key(), $i->offsetGet($i->key()));
      $i->next();
    }
    var_dump($i->getArrayCopy());
  }
}
