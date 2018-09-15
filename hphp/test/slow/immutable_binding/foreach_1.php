<?hh // experimental

function foo(array<int> $arr): void {
  $sum = 0;
  foreach ($arr as element) {
    echo "adding ";
    var_dump(element);
    $sum += element;
  }
  echo "sum is ";
  var_dump($sum);
}

foo(array());
foo(array(1));
foo(array(1, 2, 3, 4, 5));
