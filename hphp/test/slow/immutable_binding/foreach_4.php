<?hh // experimental

function foo(Map<int, int> $arr): void {
  $sum = 0;
  foreach ($arr as idx => element) {
    echo "adding idx = ".idx." ";
    var_dump(element);
    $sum += element;
  }
  echo "sum is ";
  var_dump($sum);
}

foo(Map {});
foo(Map {1 => 1});
foo(Map {1 => 1, 2 => 2, 3 => 3, 4 => 4, 5 => 5});
