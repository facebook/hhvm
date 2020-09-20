<?hh

function test($arr) {
  echo "\n\n----------------------------------------\n";
  var_dump($arr);
  echo "*** without legacy bit ***\n";
  echo json_encode($arr) . "\n";
  echo "*** with legacy bit ***\n";
  $arr = HH\array_mark_legacy($arr);
  echo json_encode($arr) . "\n";
}

<<__EntryPoint>>
function main() {

  $arrays = vec[
    vec[],
    dict[],
    vec[1, 2, 3],
    dict[0 => 1, 1 => 2, 2 => 3],
    dict["hello" => 42, "baz" => 100],
    dict[1000 => "no", "potato" => vec[]],
    ];

  foreach ($arrays as $arr) {
    test($arr);
  }
}
