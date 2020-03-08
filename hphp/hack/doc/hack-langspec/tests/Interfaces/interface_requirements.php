<?hh // strict

namespace NS_interface_requirements;

class C1 {
  public function cf1(): void { }
}

interface I1 {
  require extends C1;
  public function if1(): void;
  require extends C1;		// redundant but permitted
}

class C2 extends C1 implements I1 {
  public function if1(): void { }
}
