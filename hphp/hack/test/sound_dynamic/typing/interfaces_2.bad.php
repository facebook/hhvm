<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

// If an interface extends another interface, then it can support dynamic
// if and only if the extended interface supports dynamic

<<__SupportDynamicType>>
interface I1 {}
interface I2 extends I1 {}
