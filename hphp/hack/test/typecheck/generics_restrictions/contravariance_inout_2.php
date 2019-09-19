<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

class C<-T> {
  public function __construct(private T $item) { }
  public function bar():(function(inout T):void) {
    return (inout T $x) ==> { $x = $this->item; };
  }
}

function expectCstring(C<string> $cs):string {
  $s = "hahahahaha";
  ($cs->bar())(inout $s);
  return $s;
}

<<__EntryPoint>>
function breakit():void {
  $c = new C<mixed>(3);
  $s = expectCstring($c);
}
