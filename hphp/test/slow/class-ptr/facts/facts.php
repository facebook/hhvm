<?hh

class C { public function __construct() { echo nameof static."\n"; }}
class D extends C {}
// class E extends C {} // TODO(T227568155)
class F extends D {}

function v($a): void {
  foreach ($a as $c) {
    new $c();
  }
}

<<__EntryPoint>>
function main(): void {
  echo "===== subtypes =====\n";
  $children = HH\Facts\subtypes(C::class);
  v($children);

  echo "===== transitive_subtypes =====\n";
  $all_children = HH\Facts\transitive_subtypes(C::class);
  v($all_children);

  echo "===== supertypes =====\n";
  $parents = HH\Facts\supertypes(F::class);
  v($parents);
}
