<?hh

function f() {
  concurrent {
    $a = 42 + await f1();
    $b = 43 + await f2();
  }
}
