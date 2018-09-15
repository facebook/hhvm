<?hh

<<__EntryPoint>>
function main() {
  $inputs = vec[
    varray[1, 2, 3],
    darray['foo' => 1, 'bar' => 3, 'baz' => 5],
  ];

  $null_obj = new ArrayObject();
  var_dump($null_obj->getArrayCopy());

  foreach ($inputs as $arr) {
    $obj = new ArrayObject($arr);
    var_dump($obj->getArrayCopy() === $arr);
  }
}
