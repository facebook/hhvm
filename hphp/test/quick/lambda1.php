<?hh

<<__EntryPoint>> function foo(): void {
  $y = "asd";
  array_map(
    $k ==> { echo $k . $y . "\n"; },
    array(1,2,3,4)
  );
}
