<?hh // strict

namespace Test;

function f(): void {
  invariant(true, 'hi');
}

function g(): void {
  invariant_violation('hi');
}

function h(): void {
  fun('\Test\f');
}
