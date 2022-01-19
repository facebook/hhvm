<?hh

class :x {
  attribute ~enum {'hello'} e;
  public function __construct(
    dict<string, ?supportdynamic> $attributes,
    vec<?supportdynamic> $children,
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
