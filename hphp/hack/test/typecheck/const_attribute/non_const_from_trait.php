<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

trait T { public int $p = 1; }
<<__Const>> class C { use T; }
