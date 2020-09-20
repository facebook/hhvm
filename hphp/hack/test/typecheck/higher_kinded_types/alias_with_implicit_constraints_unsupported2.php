<?hh

// This is like alias_with_impliciit_constraints_unsupported2.php, but goes
// via an additional level of indirection by using an alias in the definiton of
// WithImplicitConstraint

class TakesNum<T1 as num> {}

class TakesHK<T2<_>> {}

type Indirection<T3> = TakesNum<T3>;

type WithImplicitConstraint<T4> = Indirection<T4>;

function test() : void {
  // This type must be rejected: We cannot use WithImplicitConstraint as a
  // HK type, because it implicitly imposes T as num on its parameter
  $x = new TakesHK<WithImplicitConstraint>();
}
