<?hh

function f(): Continuation<int> {
  {
    // UNSAFE
  }
  yield 1;
}
