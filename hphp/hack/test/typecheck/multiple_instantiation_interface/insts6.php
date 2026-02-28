<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

<<__UNSAFE_AllowMultipleInstantiations>>
interface J<+T> {}
interface I1 extends J<int> {}
interface I2 extends J<num> {}

// Legal (cos J<int> <: J<num>)
interface K extends J<int>, I2 {}

// Illegal
interface L extends J<num>, J<int> {}
