<?hh

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

function testing_area_for_shapes(): void {
  $point1 = shape('x' => -3, 'y' => 6);

}

function call_lambda(int $n, (function(int): int) $param): void {
  $param($n);
}
function testing_area_for_lambdas(): void {
  $mylambda = ($n) ==> $n * 5;

}

function test_sig_function(string $foo): void {

}

function test_call_signature(): void {
  test_sig_function("");
}
