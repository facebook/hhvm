<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

interface IA {
 const string ONCALL = "a";
}

interface IB {
  const string ONCALL = "b";
}

class A implements IA {}

final class B extends A implements IB {}
