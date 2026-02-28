<?hh

<<file:__EnableUnstableFeatures('type_const_multiple_bounds')>>

// This test exhibits a divergence in typing_escape.ml:
// When a lambda returns an expression-dependent type whose type constant
// has a self-referential bound (this::TEnt in type parameter position)
// AND another non-self-referential bound (creating an intersection),
// the refresh_type/eliminate mutual recursion loops infinitely.
//
// With only a single self-referential bound, the checker terminates
// (though slowly). Adding a second bound creates an intersection
// that prevents cycle detection from working.

interface IBase {}

interface ICreatable<+TEnt as IBase> {
  public function get(): TEnt;
}

abstract class Builder {
  abstract const type TEnt as IBase as ICreatable<this::TEnt>;

  public function getTEnt(): this::TEnt {
    throw new Exception('stub');
  }
}

function use_builder(Builder $b): void {
  $fn = (Builder $b2) ==> $b2->getTEnt();
  $fn($b);
}
