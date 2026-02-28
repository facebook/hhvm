<?hh
class A1 {
  private function bar() :mixed{ echo "A1::bar\n"; }
}
class B1 extends A1 {
  protected function bar() :mixed{ echo "B1::bar\n"; }
  public function baz() :mixed{ $this->bar(); }
}
class C1 extends B1 {
  protected function bar() :mixed{ echo "C1::bar\n"; }
}

class A2 {
  private function bar() :mixed{ echo "A2::bar\n"; }
}
class B2 extends A2 {
  public function bar() :mixed{ echo "B2::bar\n"; }
  public function baz() :mixed{ $this->bar(); }
}
class C2 extends B2 {
  public function bar() :mixed{ echo "C2::bar\n"; }
}

class A3 {
  private function bar() :mixed{ echo "A3::bar\n"; }
}
class B3 extends A3 {
  protected function bar() :mixed{ echo "B3::bar\n"; }
  public function baz() :mixed{ $this->bar(); }
}
class C3 extends B3 {
  public function bar() :mixed{ echo "C3::bar\n"; }
}

class A4 {
  private function bar() :mixed{ echo "A4::bar\n"; }
}
class B4 extends A4 {
  public function baz() :mixed{ $this->bar(); }
}

<<__EntryPoint>> function main(): void {
$obj = new C1;
$obj->baz();
$obj = new C2;
$obj->baz();
$obj = new C3;
$obj->baz();
$obj = new B4;
$obj->baz();
}
