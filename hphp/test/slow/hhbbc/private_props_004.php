<?hh

class Other {}



class MVecFinalOps {
  private $s = "this is a string";
  private $o = null;
  private $igen = 42.0;
  private $icell = 2;

  public function __construct() {
    $this->o = new Other;
    $this->icell = "asd";
  }

  public function foo() :mixed{
    var_dump(isset($this->s));
    var_dump(!($this->s ?? false));

    $this->icell = HH\Lib\Legacy_FIXME\cast_for_arithmetic($this->icell);

    $this->icell += 2;
    $this->icell++;
  }

  public function printer() :mixed{
    $s = $this->s;
    $o = $this->o;
    $igen = $this->igen;
    $icell = $this->icell;
    var_dump($s, is_string($s));
    var_dump($o, is_object($o));
    var_dump($igen);
    var_dump($icell);
  }
}

<<__EntryPoint>>
function main_private_props_004() :mixed{
  $x = new MVecFinalOps();
  $x->foo();
  $x->printer();
}
