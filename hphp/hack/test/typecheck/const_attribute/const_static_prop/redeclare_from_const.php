<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

class C { <<__Const>> static protected int $p = 1; }
class D extends C { static public int $p = 2; }
