<?hh

<<__EntryPoint>> function foo(): void {
  $y = "asd";
  array_map(
    $k ==> { echo $k . $y . "\n"; },
    varray[1,2,3,4]
  );
}
