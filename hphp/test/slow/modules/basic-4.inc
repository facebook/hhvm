<?hh

<<file:__EnableUnstableFeatures("modules")>>

module B;

interface I {
  static function foo();
}
internal class C implements I {
  static function foo() { echo "in foo\n"; }
}

function conjureC(): I {
  return new C;
}
