<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

class C { protected static int $p = 1; }
class D extends C { <<__Const>> public static int $p = 2; }
