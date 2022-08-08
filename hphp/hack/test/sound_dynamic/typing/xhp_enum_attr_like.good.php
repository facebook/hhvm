<?hh

class :x {
  attribute ~enum {'hello'} e;
  public function __construct(
    dict<string, supportdyn<mixed>> $attributes,
    vec<supportdyn<mixed>> $children,
    private string $file,
    private int $line,
  ) {}
}

<<__SupportDynamicType>>
function f(): ~string {
  return 'hello';
}

function test(): void {
  <x e={f()}/>;
}
