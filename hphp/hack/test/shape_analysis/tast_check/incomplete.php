<?hh

class :div {
  public function __construct(
    dict<string, mixed> $attributes,
    vec_or_dict<mixed> $children,
    string $file,
    int $line
  ): void {}
}

class C {
  public function f(C $c): void {
    // XHP is not yet supported
    <div />;
  }

  public function g(): void {
    dict['a' => 42, 'b' => true];
  }
}
