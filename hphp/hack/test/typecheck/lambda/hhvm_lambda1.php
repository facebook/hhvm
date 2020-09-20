<?hh // partial

function foo() {
  $y = "asd";
  array_map(
    $k ==> {
      echo $k.$y."\n";
    },
    varray[1, 2, 3, 4],
  );
}
foo();
