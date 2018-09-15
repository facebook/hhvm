<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

class D<Tu,Tv> {
  public function __construct(private Tu $item1, private Tv $item2) { }
}

function test(string $s, int $i): void {
  $d = new D<int>($i, $s);
}
