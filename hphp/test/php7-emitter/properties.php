<?php

function h($b) {
  return $b;
}

class C {
  public $a = "a";
  public static $b = "static b";
  protected $c = "protected c";
  protected static $d = "protected static d";
  private $e = "private e";
  private static $f = "private static f";

  public $z = 1;
  public $y = array(1, 2);
  public $x = array(1, 'a' => 2, true);
  public $w = 2 + 2;
  public $v = 4 > 2;
  //public $u = (1 & 2) && true;
  public $t = false;
  public $s = null;
  public $r = 5.123;
  public $q = array(1, 5 => 2, true);
  public $p = array(1, 5 => 2, true, 4 => 2, false);

  public function hello() {
    echo "hello\n";
  }

  public function weirdness() {
    echo h($this->a)."\n";
    $this->hello();
    $b = &$this;
    h($this);

    $x = "hello";
    C::$x();

    $x = "b";
    echo self::$$x."\n";

    return $this++;
  }

  public function g() {
    echo $this->a."\n";
    echo $this->c."\n";
    echo $this->e."\n";

    echo self::$b."\n";
    echo self::$d."\n";
    echo self::$f."\n";

    $this->a = 'modfified g a';
    $this->c = 'modfified g c';
    $this->e = 'modfified g e';

    self::$b = 'static modfified g b';
    self::$d = 'static modfified g d';
    self::$f = 'static modfified g f';

    echo $this->a."\n";
    echo $this->c."\n";
    echo $this->e."\n";

    echo self::$b."\n";
    echo self::$d."\n";
    echo self::$f."\n";
  }
}

class D extends C {
  public function f() {
    echo $this->a."\n";
    echo $this->c."\n";

    echo parent::$b."\n";
    echo parent::$d."\n";

    $this->a = 'modfified f a';
    $this->c = 'modfified f c';

    parent::$b = 'static modfified f b';
    parent::$d = 'static modfified f d';

    echo $this->a."\n";
    echo $this->c."\n";

    echo parent::$b."\n";
    echo parent::$d."\n";
  }
}

$obj = new D;

echo $obj->a."\n";
echo C::$b."\n";

$obj->n = "hello";

$obj->a = 'modfified a';
C::$b = 'static modfified b';

echo $obj->a."\n";
echo C::$b."\n";

$obj->f();
$obj->g();

$g = array($obj, 1);

$obj = new C;
$obj->weirdness();
echo $obj->a."\n";
$obj->g();
echo $obj->a."\n";

$h = true;
C::$b = $h ? C::$b : "asdf";

echo C::$b."\n";

echo C::$c."\n";
