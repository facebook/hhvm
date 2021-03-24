<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

// If an interface extends another interface, then it can support dynamic
// if and only if the extended interface supports dynamic

<<__SoundDynamicCallable>>
interface I1 {}
interface I2 extends I1 {}             // this is an error

interface I3 {}
<<__SoundDynamicCallable>>
interface I4 extends I3 {}    // this is also an error
