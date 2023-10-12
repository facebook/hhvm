<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

trait T { public static int $p = 1; }
<<__Const>> class C { use T; }
