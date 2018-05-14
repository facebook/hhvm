<?hh // strict

// For each pair of functions, the Ast should have the same structure, i.e.
// no extra Ast.Block should be inside the Ast

function f1(): void {
  if (true) {}
}

function f2(): void {
  if (true);
}

function g1(): void {
  if (true) func();
}

function g2(): void {
  if (true) { func(); }
}

// expect block
function g3(): void {
  if (true) { { func(); } }
}

function h1(): void {
  if (true) { func(); } else func();
}

function h2(): void {
  if (true) { func(); } else { func(); }
}

// expect block
function h3(): void {
  if (true) { func(); } else { { func(); } }
}
