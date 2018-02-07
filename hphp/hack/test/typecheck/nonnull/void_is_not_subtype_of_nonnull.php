<?hh // strict

function f(): void {}

function g(): nonnull {
  return f();
}
