<?hh

class NonException {}

function f(): void {
  throw new NonException();
}
