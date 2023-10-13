<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class C { <<__Const>> protected int $p = 1; }
trait T { public int $p = 5; }
class D extends C { use T; }
