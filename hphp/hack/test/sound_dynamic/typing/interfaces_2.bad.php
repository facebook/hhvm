<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

// If an interface extends another interface, then it can extend dynamic
// if and only if the extended interface extends dynamic

interface I1 extends dynamic {}
interface I2 extends I1 {}             // this is an error

interface I3 {}
interface I4 extends I3, dynamic {}    // this is also an error
