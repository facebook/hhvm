<?hh

function callee()[local]: void {}

function good_caller()[local]: void {
  $l_unrelated = ()[] ==> {};
  // regression test to verify that capabilities are restored
  // _after_ type-checking lambda; they shouldn't leak into enclosing scope
  callee();

  {
    $l_unrelated = ()[rx] ==> {};
    callee();
  }
}

function bad_caller()[]: void {
  $l_unrelated = ()[local] ==> {};
  // test the inverse case than above; capabilities should _not_ persist
  // _after_ type-checking the lambda, therefore the call should be disallowed
  callee();
}
