<?hh

function complete(): void {
  dict['a' => 42];
}

class :div {
  public function __construct(
    dict<string, mixed> $attributes,
    vec_or_dict<mixed> $children,
    string $file,
    int $line
  ): void {}
}

function incomplete(): void {
  <div />;
  dict['a' => 42];
  <div />;
}
