<?hh // strict

function f ((function (...) : int) $g) : int {
  g(1, 2, 3);
}
