<?hh

<<__EntryPoint>> function foo(): void {
  $y = 100;
  $k = () ==> {
    echo $y . "\n";
    return $y ==> {
      $z = () ==> $y;
      return $z() + 1;
    };
  };
  $l = $k();
  echo $l(42);
  echo "\n";
}
