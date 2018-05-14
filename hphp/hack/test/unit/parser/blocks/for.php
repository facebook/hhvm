<?hh // strict

// For each pair of functions, the Ast should have the same structure, i.e.
// no extra Ast.Block should be inside the Ast

function f1(): void {
  for (;;);
}

function f2(): void {
  for (;;) {}
}

function g1(): void {
  for (;;) func();
}

function g2(): void {
  for (;;) { func(); }
}

// expect block
function g3(): void {
  for (;;) { { func(); } }
}
