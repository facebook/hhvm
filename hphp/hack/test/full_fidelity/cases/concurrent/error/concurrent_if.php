<?hh

function f() {
  concurrent {
    $a = await f1();
    if (await f2()) {
      $x = 1;
    } else {
      $x = 2;
    }
  }
}
