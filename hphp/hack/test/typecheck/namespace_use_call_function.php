<?hh

namespace N\NN {
  function f(): void {}
}

namespace N2 {
  use N\NN;
  function f(): void {
    NN\f();
  }
}
