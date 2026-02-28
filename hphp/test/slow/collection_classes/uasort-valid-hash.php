<?hh

<<__EntryPoint>>
function main() :mixed{
  $m = Map{'a' => 3, 'b' => 2, 'c' => 1};
  uasort(inout $m, ($a, $b) ==> {
    echo $m['a'] . ' ' . $m['b'] . ' ' . $m['c'] . "\n";
    return $a <=> $b;
  });
  var_dump($m);
}
