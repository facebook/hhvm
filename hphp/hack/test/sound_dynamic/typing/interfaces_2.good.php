<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

// If an interface extends another interface, then it can support dynamic
// if and only if the extended interface support dynamic

<<__SoundDynamicCallable>>
interface I1 {}
<<__SoundDynamicCallable>>
interface I2 extends I1 {}   // this is OK
