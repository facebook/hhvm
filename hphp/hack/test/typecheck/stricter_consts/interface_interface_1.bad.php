<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

// Error: interfaces conflict

interface IA {
 const string ONCALL = "a";
}

interface IB {
  const string ONCALL = "b";
}

class A implements IA, IB {}
