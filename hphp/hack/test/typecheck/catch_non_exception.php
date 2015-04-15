<?hh // strict

class NonException {}

function f(): void {
  try {
  } catch (NonException $m) {
  }
}
