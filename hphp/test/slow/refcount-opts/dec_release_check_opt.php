<?hh


<<__EntryPoint>> function main(): void {
  $x = vec[-1, 3, 6, 7];
  $a = 100;
  $cls = ($b) ==> {
    $c = $a + $b;
    var_dump($c);
  };
  foreach ($x as $v) {
    $cls($v);
  }
}
