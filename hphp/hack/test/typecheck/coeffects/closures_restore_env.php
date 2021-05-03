<?hh

function callee()[write_props]: void {}

function good_caller()[write_props]: void {
  $l_unrelated = ()[] ==> {};
  // regression test to verify that capabilities are restored
  // _after_ type-checking lambda; they shouldn't leak into enclosing scope
  callee();

  {
    $l_unrelated = ()[globals] ==> {};
    callee();
  }
}

function bad_caller()[]: void {
  $l_unrelated = ()[write_props] ==> {};
  // test the inverse case than above; capabilities should _not_ persist
  // _after_ type-checking the lambda, therefore the call should be disallowed
  callee();
}
