<?hh

<<__EntryPoint>>
function f(): void {
  $result = readonly dict[];
  $result[2] ??= 42;
  var_dump($result);

  $result = readonly dict[];
  foreach (vec[1, 2, 3] as $i) {
    $result[$i] ??= vec[];
  }
  var_dump($result);
}
