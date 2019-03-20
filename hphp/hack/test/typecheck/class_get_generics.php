<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

class D<T> {
  public static int $i = 4;
}

function f(): void {
  D<string>::$i;
}
