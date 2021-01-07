<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

// If an interface extends another interface, then it can extend dynamic
// if and only if the extended interface extends dynamic

interface I1 extends dynamic {}
interface I2 extends I1, dynamic {}   // this is OK
