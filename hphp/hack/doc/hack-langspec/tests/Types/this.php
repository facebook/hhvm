<?hh // strict

namespace NS_this;

trait TR {

// constants

  abstract const ?this TRM1a;
  const ?this TRM1c = null;

// methods

  abstract public function TRf(this $val): this;
}

interface I {

// constants

  abstract const ?this IM1a;
  const ?this IM1c = null;

// methods

  public function inf(this $val): this;

// type constants

  abstract const type IT1 as this;
}

abstract class C implements I {

// constants

  abstract const ?this M1a;
  const ?this M1c = null;

  const ?this IM1a = null;

// properties

//  static private this $p1s;	// The type "this" cannot be used as the type of a static member variable
//  static private ?this $p2s;	// The type "this" cannot be used as the type of a static member variable

  private string $name;

  private ?this $p1c;
  protected ?this $p2c;
  public ?this $p3c;

// constructor

  public function __construct(string $name) {
    echo "Inside " . __METHOD__ . "\n";
    $this->name = $name;
    var_dump($this); 
    $this->setProp($this);
    $this->p2c = $this->getProp();
    var_dump($this->p2c); 
  }

// methods

  public function setProp(this $val): void {
    echo "Inside " . __METHOD__ . "\n";
    var_dump($val); 
    $this->p1c = $val;
  }

  public function getProp(): ?this {
    echo "Inside " . __METHOD__ . "\n";
    var_dump($this->p1c); 
    return $this->p1c;
  }

  private function privf(this $val): this { return $val; }
  private static function privsf(this $val): this { return $val; }
  protected function protf(this $val): this { return $val; }
  protected static function protsf(this $val): this { return $val; }
  public function pubf(this $val): this { return $val; }
  public static function pubsf(this $val): this {
    echo "Inside " . __METHOD__ . "\n";
    var_dump($val); 
    return $val;
  }
  public function inf(this $val): this { return $val; }

// type constants

  abstract const type T1 as this;
}

class D extends C {
  const ?this M1a = null;
  const type T1 = this;
  const type IT1 = this;
}

class E extends D {}

//class G<T as this> {}		// The type "this" cannot be used as a constraint on a generic class 
//class G<T as ?this> {}	// The type "this" cannot be used as a constraint on a generic class 

function main(): void {
  $d = new D("AA");
  var_dump($d);
  D::pubsf(new D("BB"));
  E::pubsf(new E("CC"));
}

/* HH_FIXME[1002] call to main in strict*/
main();