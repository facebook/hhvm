<?hh

class A {
  public function foo(int $x): void {
    // The extracted method should *NOT* contain snippets (like `${0:placeholder}`),
    // because we are passing `--ide-code-actions-no-experimental-capabilities` to simulate
    // a client that doesn't have client experimental capability `snippetTextEdit`
    /*range-start*/
    $y = $x + 1;
    $z = $y + 1;
    /*range-end*/
    $zz = $z;
  }
}
