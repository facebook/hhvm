<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

interface IA {
 const string ONCALL = "a";
}
trait TIA implements IA {}


interface IB {
  const string ONCALL = "b";
}
trait TIB implements IB {}

class A {
  use TIA;
}

final class B extends A {
 use TIB;
}
