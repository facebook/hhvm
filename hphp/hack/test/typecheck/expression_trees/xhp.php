<?hh

<<file: __EnableUnstableFeatures('expression_trees')>>

class :my:xhp {
  public function __construct(
    dict<string, mixed> $attrs,
    vec<mixed> $children,
    string $file,
    int $line,
  ) {}
}

function foo(): void {
  $x = ExampleDsl`<my:xhp />`;
}
