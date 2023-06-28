<?hh

function test($arr) :mixed{
  echo "\n\n----------------------------------------\n";
  var_dump($arr);
  echo "*** without legacy bit ***\n";
  echo serialize($arr) . "\n";
  echo "*** with legacy bit ***\n";
  $arr = HH\array_mark_legacy($arr);
  echo serialize($arr) . "\n";
  echo HH\serialize_with_options($arr, dict['keepDVArrays' => true]) . "\n";
  echo "*** after a CoW ***\n";
  $arr[] = "blarghghg!";
  echo serialize($arr) . "\n";
  echo HH\serialize_with_options($arr, dict['keepDVArrays' => true]) . "\n";
}

<<__EntryPoint>>
function main() :mixed{

  $arrays = vec[
    vec[],
    dict[],
    vec[1, 2, 3],
    dict["hello" => 42, "baz" => 100],
    dict[1000 => "no", "potato" => vec[]],
  ];

  foreach ($arrays as $arr) {
    test($arr);
  }
}
