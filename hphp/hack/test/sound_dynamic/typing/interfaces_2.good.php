<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

// If an interface extends another interface, then it can support dynamic
// if and only if the extended interface support dynamic

<<__SupportDynamicType>>
interface I1 {}
<<__SupportDynamicType>>
interface I2 extends I1 {}   // this is OK

interface I3 {}
<<__SupportDynamicType>>
interface I4 extends I3 {}   // this is also OK
