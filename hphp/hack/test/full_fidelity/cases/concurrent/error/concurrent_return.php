<?hh

function f() {
  concurrent {
    $a = await f1();
    return await f2();
  }
}
