<?hh

class AttributeForTesting implements HH\MethodAttribute {
  public function __construct(mixed $anything) {}
}

function test(
  <<AttributeForTesting(42)>> mixed ... $_
): void {}
