<?hh // strict

function foo(): void {
  $a = 10;
  $b = $a ** 10; // num
  $c = 3 ** $b; // num
  hh_show($b);
  hh_show($c);

  $d = ($c === $a ** $b ** $c);
  hh_show($d);

  $e = ((int)$d) ** 3.5;
  hh_show($e);
  $e = $e ** 100000;
  hh_show($e);

  $i = 1;
  $f = 3.5;

  $a = 1 ** 1.0; // float
  hh_show($a);
  $b = 1.0 ** 1; // float
  hh_show($b);
  $c = 1.0 ** 1.0; // float
  hh_show($c);

  hh_show(100000000 ** 10000000); // num
  hh_show(2 ** 2); // num
}
