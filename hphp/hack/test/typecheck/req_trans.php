<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

interface J {
  protected function bar():void;
}
abstract class AD implements J
{
  protected function bar():void { }
}
abstract class AC extends AD { }
interface I { require extends AC; }

trait TR {
  require implements I;

  public function foo():void {
    $this->bar();
  }
}
