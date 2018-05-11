<?hh //strict

function foo(): void {
  let x : int = 42;
  x = 0; // fail due to immutability
}
