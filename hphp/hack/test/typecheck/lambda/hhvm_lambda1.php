<?hh

function foo(): void {
  $y = "asd";
  array_map(
    $k ==> {
      echo $k.$y."\n";
    },
    vec[1, 2, 3, 4],
  );
}
