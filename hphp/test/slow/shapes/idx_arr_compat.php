<?hh

<<__EntryPoint>>
function main() :mixed{
  $arrs = vec[
    shape('x' => 1),
    dict['x' => 1],
    vec[true, false],
    new Map(dict['x' => 1]),
  ];
  foreach ($arrs as $arr) {
    var_dump($arr);
    try {
      var_dump(Shapes::idx($arr, 'x'));
    } catch (Exception $e) {
      var_dump($e->getMessage());
    }
  }
}
