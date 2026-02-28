<?hh

function f(): void {}

function g(): nonnull {
  return f();
}
