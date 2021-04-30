<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

// a class that implements dynamic can implement any interface
// (including interfaces that do not extend dynamic)

interface I1 {}

<<__SupportDynamicType>>
interface I2 {}

<<__SupportDynamicType>>
class D implements I1, I2 {}
