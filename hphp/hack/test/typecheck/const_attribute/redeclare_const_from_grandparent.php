<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class C { <<__Const>> protected int $p = 1; }
class D extends C {}
class E extends D { public int $p = 2; }
