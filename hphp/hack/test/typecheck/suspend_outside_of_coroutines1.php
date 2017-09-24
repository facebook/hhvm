<?hh // strict

function f($b) {
  $a = suspend $b();
}
