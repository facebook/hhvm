<?hh

function generateData() :mixed{
  return dict[
    "my_data" => Vector{3},
    "my_map" => Map{'foo' => 'bar'}
  ];
}

function accumulate() :mixed{
  $accum = null;
  for ($i = 0; $i < 3; $i++) {
    $data = generateData();
    if ($accum === null) {
      $accum = $data;
    } else {
      $accum['my_data']->addAll($data['my_data']);
      $accum['my_map']->addAll($data['my_map']->items());
    }
  }
  return $accum === null ? dict[
    "my_data" => Vector{},
    "my_map" => Map{}
  ] : $accum;
}

<<__EntryPoint>>
function main() :mixed{
  $accum = accumulate();
  var_dump($accum);
}
