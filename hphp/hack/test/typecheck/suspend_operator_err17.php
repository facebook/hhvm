<?hh // strict

coroutine function g(): int {
  $arr = array(
    () ==> 1,
    coroutine function() {
      return 2;
    },
  );
  $r = 0;
  foreach ($arr as $c) {
    // not ok - $c is not definitely known to be a regular function
    $r += $c();
  }
  return $r;
}
