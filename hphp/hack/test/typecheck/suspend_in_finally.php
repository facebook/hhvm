<?hh // strict

function f() {
  $c = coroutine function($a, $someCoroutine) {
    try {
      print 1;
    } finally {
      while ($a) {
        $a = 10 + suspend $someCoroutine();
      }
    }
  };
}
