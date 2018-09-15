<?hh // strict

coroutine function g(): int {
  // ok - argument of suspend is call to coroutine typed value
  $arr = array(
    coroutine () ==> 1,
    coroutine function() {
      return 2;
    },
  );
  $r = 0;
  foreach ($arr as $c) {
    $r += suspend $c();
  }
  return $r;
}
