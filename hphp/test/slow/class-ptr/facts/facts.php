<?hh

class C { public function __construct() { echo nameof static."\n"; }}
<<Attr>>
class D extends C {}
class E extends C {}
<<Attr>>
class F extends D {}

function v($a): void {
  foreach ($a as $c) {
    new $c();
  }
}

<<Attr>>
type G = C;

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

  echo "===== type_attributes =====\n";
  $classes = HH\Facts\types_with_attribute(Attr::class);
  v($classes);
}
