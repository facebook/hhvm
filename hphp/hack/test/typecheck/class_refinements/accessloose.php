<?hh

<<file:__EnableUnstableFeatures('type_refinements')>>

class Ref<T> {
  public function __construct(public T $data) {}
}

interface Box {
  abstract const type T;
  public function set(this::T $t): void;
  public function get(): this::T;
}

function external_get<TB as Box, T>(TB $b) : T where T = TB::T {
  return $b->get();
}

function mut_get<TB as Box, T>(Ref<TB> $b) : T where T = TB::T {
  return $b->data->get();
}

function ok1(Box with { type T as int } $b) : int {
  // Accessing on a loose refinement is undesirable
  // however, in the following example, we perform
  // the access on the expression-dependent type
  // minted to represent the runtime class of the
  // value in $b. By the loose constraint above
  // we know that $b::T <: int, and the function
  // typechecks fine.
  return $b->get();
}

function ok2(Box with { type T as int } $b) : int {
  // Same reasoning as above.
  return external_get($b);
}

function bad3(Ref<Box with { type T as int }> $mb) : int {
  // Here we are performing a type access on
  // a loose refinement since no expresion-
  // dependent type is created to reflect
  // the contents of the Ref<>; so the
  // typechecker correctly flags an error
  return mut_get($mb);
}

function bad4(Ref<Box with { type T as int }> $mb) : int {
  // Same as above, but a different code path
  // in the typechecker because no type variable
  // is created for TB on the call.
  return mut_get<Box with { type T as int }, _>($mb);
}
