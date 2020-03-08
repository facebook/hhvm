<?hh // strict

namespace NS_trait_requirements;

class C1 {
  public function cf1(): void { }
}

interface I1 {
  public function if1(): void;
}

trait T1 {
  require extends C1;
  require implements I1;
  public function f(): void {
    $this->cf1();
    $this->if1();
  }
}

class C2 extends C1 implements I1 {
  use T1;
  public function if1(): void { }
}

trait T2 {
  public function f(): void {
    $this->cf1();
    $this->if1();
  }
  require implements I1;
  require extends C1;
}

class C3 extends C1 implements I1 {
  use T2;
  public function if1(): void { }
}

/*
class Cx {
  use T1; // Error
}
*/

interface I2 {
  public function if2(): void;
}

trait T3 {
  require implements I1;
  require extends C1;		// redundant, as C3 already extends C1, but permitted
  require implements I2;
  require implements I2;	// redundant, but permitted
  require extends C3;
  require extends C3;		// redundant, but permitted
}

class C4 extends C3 implements I1, I2 {
  use T3;
  public function if1(): void { }
  public function if2(): void { }
}

/*
interface Ix {
  use Tx;
}

trait Tx {
  require implements Ix;	// cyclic definition; not permitted
}
*/

