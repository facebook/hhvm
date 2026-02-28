<?hh

class A {}

function f(
  A with {
    const type T = int
  } $_
): void {}
