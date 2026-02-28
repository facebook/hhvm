<?hh

<<__SupportDynamicType>>
class C {
  public function __construct(
    private string $s,
    private int $i,
    private string $x,
  ) {}

}

<<__SupportDynamicType>>
function newC(string $s, int $i, string $x): C {
  return new C($s, $i, $x);
}

function getString(): ~string {
  return "A";
}

function getInt(): ~int {
  return 3;
}

<<__SupportDynamicType>>
function test(): void {
  $s = getString();
  $i = getInt();
  $c = newC($s, $i, "A");
  $c = new C($s, $i, "A");
}
