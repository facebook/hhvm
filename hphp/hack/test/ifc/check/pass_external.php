<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

class A {
  public function __construct() {}
}

class B {
  <<Policied("PUBLIC")>>
  public A $pub;

  <<Policied("PRIVATE")>>
  public A $priv;

  public function __construct() {
    $this->pub = new A();
    $this->priv = new A();
  }
}

<<Governed("A")>>
function f(<<External>> A $x): void {}

<<Governed("A")>>
function g(A $x): void {}

<<Governed("A")>>
function governed_to_external(A $x): void {
  // Ok! Regular values can be passed to external because external is more
  // restrictive
  f($x);
}

<<Governed("A")>>
function external_to_governed(<<External>> A $x): void {
  // Not ok! An external cannot be used as a regular value
  g($x);
}

<<InferFlows>>
function public_to_A(B $x): void {
  // Ok! Even though f is governed by the policy A its argument is marked as
  // external and, as such, can be subject to any policy P such that P flows to
  // A. Public is such a policy, so the call is fine.
  f($x->pub);
}

<<Governed("PUBLIC")>>
function external_to_external(<<External>> A $x): void {
  try {
    // Ok because the arg is external
    f($x);
  } catch (Exception $_) {}
}

<<InferFlows>>
function private_to_A(B $x): void {
  // Not allowed because PRIVATE does not flow to A
  f($x->priv);
}

<<Governed("PRIVATE")>>
function private_external_to_A(<<External>> A $x): void {
  // Not allowed! $x can be subject to any policy P that flows to Private, that
  // is, to any policy. In particular P might not flow into A as required for
  // f's argument, so the call is invalid.
  f($x);
}
