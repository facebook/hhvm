<?

function f() {
  $a = range(-3, 3);
  $a []= true;
  $a []= false;
  $a []= 3.14;
  $a []= "123";
  $a []= "0";
  foreach ($a as $l) {
    foreach ($a as $r) {
      var_dump($l % $r);
    }
  }
}

f();

