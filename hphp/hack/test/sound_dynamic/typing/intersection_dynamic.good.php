<?hh

enum E : string as string { A = "A"; }

function getLikeE(): ~E {
  return E::A;
}

<<__SupportDynamicType>>
function foo(): (~E & string) {
  // Should succeed, because inside the function
  // we treat the return type as fully-enforced
  // and therefore allow dynamic to flow to it.
  return getLikeE();
  }

<<__SupportDynamicType>>
interface I {
  public function get():string;
}

<<__SupportDynamicType>>
class C implements I {
  public function __construct(private ~E $item) {}
  // This is allowed too
  public function get():(~E & string) { return $this->item; }
}
