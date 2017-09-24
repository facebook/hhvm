<?hh // strict

function f($b): Iterator<int> {
  yield 10;
  $a = suspend $b();
}
