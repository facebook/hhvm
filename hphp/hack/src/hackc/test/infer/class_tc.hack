interface SomeAttribute extends HH\StaticPropertyAttribute { }

class D {
  const int C = 5;
}

function sink(mixed $x): void { }
function source(): mixed { return 0; }
