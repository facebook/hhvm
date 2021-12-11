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

class :my-child implements XHPChild {
  public function __construct(
    dict<string, mixed> $attrs,
    vec<mixed> $children,
    string $file,
    int $line,
  ) {}
}

function foo(): void {
  $s = ExampleDsl`"abc"`;

  $x = ExampleDsl`() ==> {
    $y = "foo";
    $z = <my-child />;

    // We should allow all of the following as children:
    // * plain text: "literal text"
    // * XHP expressions: <my-child />
    // * interpolation: {$y}
    // * interpolating a splice: {${$s}}
    return <my:xhp> literal text <my-child /> {$y}{$z} {${$s}} </my:xhp>;
  }`;
}
