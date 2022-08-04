<?hh

<<file: __EnableUnstableFeatures('expression_trees')>>

class :my:xhp {
  attribute
    ExampleString my-str,
    ExampleInt my-number;
  public function __construct(
    dict<string, mixed> $attrs,
    vec<mixed> $children,
    string $file,
    int $line,
  ) {}
}

function foo(): void {
  $x = ExampleDsl`() ==> {
    $x = 1;
    return <my:xhp my-str="bar" my-number={$x} />;
  }`;
}
