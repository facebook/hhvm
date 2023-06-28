<?hh

<<__EntryPoint>>
function main() :mixed{
  $inputs = vec[
    varray[],
    varray[1, 2, 3],
    darray['foo' => 1, 'bar' => 3, 'baz' => 5],
  ];

  foreach ($inputs as $arr) {
    $i = new ArrayIterator($arr);
    while ($i->valid()) {
      var_dump($i->key(), $i->current());
      $i->next();
    }
    var_dump($arr);
  }
}
