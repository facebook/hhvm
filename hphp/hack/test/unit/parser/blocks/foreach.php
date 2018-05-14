<?hh // strict

// For each pair of functions, the Ast should have the same structure, i.e.
// no extra Ast.Block should be inside the Ast

function f1(array<int> $arr): void {
  foreach ($arr as $element);
}

function f2(array<int> $arr): void {
  foreach ($arr as $element) {}
}

function g1(array<int> $arr): void {
  foreach ($arr as $element) func();
}

function g2(array<int> $arr): void {
  foreach ($arr as $element) { func(); }
}

// expect block
function g3(array<int> $arr): void {
  foreach ($arr as $element) { { func(); } }
}
