<?hh

<<__EntryPoint>>
function main() :mixed{
  $m = Map{'c' => 1, 'b' => 2, 'a' => 3};
  uksort(inout $m, ($a, $b) ==> {
    echo $m['a'] . ' ' . $m['b'] . ' ' . $m['c'] . "\n";
    return $a <=> $b;
  });
  var_dump($m);
}
