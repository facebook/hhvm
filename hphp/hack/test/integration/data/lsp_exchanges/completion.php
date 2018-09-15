<?hh  //strict

function testing_area(): void {

}

/** test_function docblock. */
function test_function(): void {

}

interface CompletionInterface {
  /** Doc block should fall back to interface. */
  public function interfaceDocBlockMethod(): void;
}

class CompletionClass {
  public function interfaceDocBlockMethod(): void {}
}
