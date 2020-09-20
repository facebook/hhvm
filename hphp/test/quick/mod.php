<?hh

<<__EntryPoint>> function f(): void {
  $a = range(-3, 3);
  $a []= true;
  $a []= false;
  $a []= 3.14;
  $a []= "123";
  $a []= "0";
  foreach ($a as $l) {
    foreach ($a as $r) {
      try {
        var_dump($l % $r);
      } catch (DivisionByZeroException $e) {
        echo $e->getMessage(), "\n";
      }
    }
  }
}
