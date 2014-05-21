<?hh

function f(): Continuation<int> {
  foreach (array(1) as $x) {
    {
      // UNSAFE
    }
    yield $x;
  }
}
