<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class C { protected int $p = 1; }
class D extends C { <<__Const>> public int $p = 2; }
