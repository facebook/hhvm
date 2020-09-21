<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

<<__EnumClass>>
class A {
    const X  = new stdclass;
}

<<__EnumClass>>
class B {
    const X = A::X;
}

<<__EntryPoint>>
function enum_class08() {
    var_dump(B::X);
}
