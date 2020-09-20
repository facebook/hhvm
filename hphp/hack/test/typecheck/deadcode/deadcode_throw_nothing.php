<?hh //partial

function func(int $x) {
  // This `hh_show` is needed. We want to make sure that the type is
  // `nothing`.
  // The previous behavior of the typechecker was to give the type `any` to
  // variables in deadcode, and calling an `expect_nothing(nothing $_)`
  // function with an argument of type `any` typechecks.
  hh_show($x);
  throw new Exception('');
  hh_show($x);
}
