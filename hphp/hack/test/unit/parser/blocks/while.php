<?hh // strict

// For each pair of functions, the Ast should have the same structure, i.e.
// no extra Ast.Block should be inside the Ast

function f1(): void {
  while (true);
}

function f2(): void {
  while (true) {}
}

function g1(): void {
  while (true) func();
}

function g2(): void {
  while (true) { func(); }
}

// expect block
function g3(): void {
  while (true) { { func(); } }
}
