<?hh

<<__EntryPoint>>
function main() {
  $arrs = vec[
    __hhvm_intrinsics\dummy_cast_to_kindofarray(dict['x' => 1]),
    shape('x' => 1),
    darray['x' => 1],
    varray[true, false],
    new Map(dict['x' => 1]),
  ];
  foreach ($arrs as $arr) {
    var_dump($arr);
    var_dump(Shapes::idx($arr, 'x'));
  }
}
