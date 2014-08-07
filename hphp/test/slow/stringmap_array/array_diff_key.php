<?hh

function main() {
  $a = msarray('a' => 97, 'b' => 98);
  $b = array('a' => 65, 98 => 'b');
  $c = array_diff_key($a,
                      $b);
  var_dump($a);
  var_dump($b);
  var_dump($c);
}

main();
