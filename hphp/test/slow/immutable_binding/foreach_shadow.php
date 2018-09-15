<?hh // experimental

function foo(array<int> $arr): void {
  let element = 42;
  $sum = 0;
  foreach ($arr as element) {
    echo "adding ";
    var_dump(element);
    $sum += element;
  }
  echo "magic element is ";
  var_dump(element);
  echo "sum is ";
  var_dump($sum);
}

foo(array());
foo(array(1));
foo(array(1, 2, 3, 4, 5));
