<?hh

function f(): Generator<int, int, void> {
  foreach (array(1) as $x) {
    {
      // UNSAFE
    }
    yield $x;
  }
}
