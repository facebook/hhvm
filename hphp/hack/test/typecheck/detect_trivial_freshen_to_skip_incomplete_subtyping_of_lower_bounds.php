<?hh

<<file: __EnableUnstableFeatures('union_intersection_type_hints')>>

final class MyContainer<T> {
  public function put(T $x): void {}
  public function get(): T {
    throw new Exception();
  }
}

interface IHasMeth {
  public function meth(): int;
}

interface A extends IHasMeth {}
interface B extends IHasMeth {}
interface C extends IHasMeth {}
interface D extends IHasMeth {}

// (A & ((B & C) | D)) is special because in our current subtyping logic
// (A & ((B & C) | D)) <: (A & ((B & C) | D)) fails due to incompleteness
function myfoo((A & ((B & C) | D)) $x): void {
  $c = new MyContainer();
  $c->put($x);
  $y = $c->get();
  // $y : tyvar w/ lower bound: A & ((B & C) | D)
  $y->meth(); // forces solve
}
