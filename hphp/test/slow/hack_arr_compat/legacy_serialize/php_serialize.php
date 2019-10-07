<?hh

function test($arr) {
  echo "\n\n----------------------------------------\n";
  var_dump($arr);
  echo "*** without legacy bit ***\n";
  echo serialize($arr) . "\n";
  echo "*** with legacy bit ***\n";
  $arr = HH\mark_legacy_hack_array($arr);
  echo serialize($arr) . "\n";
  echo "*** after a CoW ***\n";
  $arr[] = "blarghghg!";
  echo serialize($arr) . "\n";

}

<<__EntryPoint>>
function main() {

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
