<?hh

class :div {
  public function __construct(
    dict<string, mixed> $attributes,
    vec_or_dict<mixed> $children,
    string $file,
    int $line
  ): void {}
}

function f(): void {
  dict['a' => 42];
  <div />; // XML is not yet supported
  $d = dict['b' => 42];
  <div />; // XML is not yet supported
  if (42 === 24) {
    $d['c'];
  } else {
    $d['d'];
  }
  inspect($d);
}
