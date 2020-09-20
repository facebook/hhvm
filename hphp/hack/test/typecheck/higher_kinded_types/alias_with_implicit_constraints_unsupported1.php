<?hh

class TakesNum<T as num> {}

class TakesHK<T<_>> {}

type WithImplicitConstraint<T> = TakesNum<T>;

function test() : void {
  // This type must be rejected: We cannot use WithImplicitConstraint as a
  // HK type, because it implicitly imposes T as num on its parameter
  $x = new TakesHK<WithImplicitConstraint>();
}
