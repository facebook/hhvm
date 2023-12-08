<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

interface J<+T> { }
interface I1 extends J<int> { }
interface I2 extends J<num> { }

// Legal (cos J<int> <: J<num>)
interface K extends I1, I2 { }

// Illegal
interface L extends I2, I1 { }

function expectJInt(J<int> $_):void { }
function test(L $l):void {
  expectJInt($l);
  }
