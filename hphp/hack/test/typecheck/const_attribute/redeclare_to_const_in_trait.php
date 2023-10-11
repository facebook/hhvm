<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class C { protected int $p = 1; }
trait T { <<__Const>> public int $p = 5; }
class D extends C { use T; }
