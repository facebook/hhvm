<?hh // partial
// Copyright 2004-present Facebook. All Rights Reserved.
trait T {
  public function __construct(private int $x) {}
}

interface I {
  public function __construct(private int $x);
}
