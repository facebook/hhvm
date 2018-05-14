<?hh // strict

// For each pair of functions, the Ast should have the same structure, i.e.
// no extra Ast.Block should be inside the Ast

function f(): void {
  do {} while (true);
}

function g1(): void {
  do func(); while (true);
}

function g2(): void {
  do { func(); } while (true);
}

// expect block
function g3(): void {
  do { { func(); } } while (true);
}
