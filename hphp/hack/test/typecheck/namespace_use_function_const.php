<?hh

namespace N {
  function f(): void {
    echo "hi\n";
  }

  const int C = 1;
}

namespace N2 {
  use function N\f;
  use const N\C as D;

  function f2(): int {
    f();
    return D;
  }
}
