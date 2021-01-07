<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

// a class that implements dynamic can implement any interface
// (including interfaces that do not extend dynamic)

interface I1 {}

interface I2 extends dynamic {}

class D implements I1, I2, dynamic {}
