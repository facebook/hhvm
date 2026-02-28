<?hh

class NonException {}

function might_throw(): void {}

function f(): void {
  try {
    might_throw();
  } catch (NonException $m) {
  }
}
