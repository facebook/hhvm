<?hh // experimental

function foo(Vector<int> $arr): void {
  $sum = 0;
  foreach ($arr as element) {
    echo "adding ";
    var_dump(element);
    $sum += element;
  }
  echo "sum is ";
  var_dump($sum);
}

foo(Vector{});
foo(Vector{1});
foo(Vector{1, 2, 3, 4, 5});
