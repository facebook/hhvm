<?hh // strict

async function asyncFunction() {
  $a = coroutine function(AsyncIterator<string> $x) {
    foreach ($x await as $v) {
      print $x;
    }
  };
}
