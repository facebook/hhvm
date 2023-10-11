<?hh

function expect<T as int>(?T $_): void {}

function test<T>(?T $x): void {
  expect($x);
}
