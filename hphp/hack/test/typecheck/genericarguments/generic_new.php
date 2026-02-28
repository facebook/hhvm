<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class C<T> {
  public function __construct(private T $item) { }
}
class D<Tu,Tv> {
  public function __construct(private Tu $item1, private Tv $item2) { }
}

function test(string $s, int $i): void {
  $a = new C<string>($s);
  $b = new C<mixed>($s);
  $c = new C<int>($i);
  $d = new D<int,string>($i, $s);
  $e = new D<arraykey, arraykey>($i, $s);
  $f = new D<_, string>($i, $s);
}
