<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function foo(I $i): void {
  $x = $i->getJ();
  $x->bar($i);
}

interface I {
  public function getJ(): J<this>;
}

interface J<-Tj as I> {
  public function bar(Tj $x): void;
}
