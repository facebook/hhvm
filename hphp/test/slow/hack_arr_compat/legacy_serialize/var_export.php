<?hh

function test($arr) :mixed{
  echo "\n\n----------------------------------------\n";
  var_dump($arr);
  echo "*** without legacy bit ***\n";
  echo var_export($arr, true) . "\n";
  echo "*** with legacy bit ***\n";
  $arr = HH\array_mark_legacy($arr);
  echo var_export($arr, true) . "\n";
}

<<__EntryPoint>>
function main() :mixed{

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
