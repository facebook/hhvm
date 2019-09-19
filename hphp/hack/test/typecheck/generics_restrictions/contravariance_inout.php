<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

class C<-T> {
  public function __construct(private T $item) { }
  public function foo(inout T $x):void {
    $x = $this->item;
  }
}

function expectCstring(C<string> $cs):string {
  $s = "hahahahaha";
  $cs->foo(inout $s);
  return $s;
}

<<__EntryPoint>>
function breakit():void {
  $c = new C<mixed>(3);
  $s = expectCstring($c);
}
