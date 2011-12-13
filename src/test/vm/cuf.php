<?php
error_reporting(-1);

class A {
  public function foo() {
    echo __CLASS__ . ' ' . get_called_class() . ' ' .
         (isset($this) ? get_class($this) : '') . "\n";
  }
  public static function bar() {
    echo __CLASS__ . ' ' . get_called_class() . ' ' .
         (isset($this) ? get_class($this) : '') . "\n";
  }
}

class B extends A {
  public function foo() {
    echo __CLASS__ . ' ' . get_called_class() . ' ' .
         (isset($this) ? get_class($this) : '') . "\n";
  }
  public static function bar() {
    echo __CLASS__ . ' ' . get_called_class() . ' ' .
         (isset($this) ? get_class($this) : '') . "\n";
  }
  public function doFoo() {
    $this->foo();  // B B B
    B::foo();      // B B B
    C::foo();      // C C   (Zend outputs: C B B) (Rule 1)
    D::foo();      // D D   (Zend outputs: D B B) (Rule 1)
    F::foo();      // F F   (Zend outputs: F B B) (Rule 1)
    G::foo();      // G G   (Zend outputs: G B B) (Rule 1)
    H::foo();      // H H   (Zend outputs: H B B) (Rule 1)
    parent::foo(); // A B B
    self::foo();   // B B B
    static::foo(); // B B B
    echo "****************\n";
  }
  public function doBar() {
    $this->bar();  // B B
    B::bar();      // B B
    C::bar();      // C C
    D::bar();      // D D
    F::bar();      // F F
    G::bar();      // G G
    H::bar();      // H H
    parent::bar(); // A B
    self::bar();   // B B
    static::bar(); // B B
    echo "****************\n";
  }
}

class C extends B {
  public function foo() {
    echo __CLASS__ . ' ' . get_called_class() . ' ' .
         (isset($this) ? get_class($this) : '') . "\n";
  }
  public static function bar() {
    echo __CLASS__ . ' ' . get_called_class() . ' ' .
         (isset($this) ? get_class($this) : '') . "\n";
  }
  /**
   * Zend's implementation of call_user_func() and forward_static_call() has
   * a few corner cases where its behavior is strictly incorrect. HipHop will
   * match Zend's behavior except for cases that violate one of the following
   * invariants:
   *   1) If the current instance ($this) is non-null, the class of the current
   *      instance must be the same or a descendent of the class enclosing the
   *      current method. The PHP group is in the process of depracating this
   *      behavior. HipHop explicitly lists this in the inconsistencies doc.
   *   2) If the current instance ($this) is non-null, the class of the current
   *      instance must be the same as the current late bound class (aka the
   *      "called class"). This invariant is stated explicitly in Zend PHP's
   *      documentation: "In non-static contexts, the called class will be the
   *      class of the object instance."
   *   3) Normal style calls and call_user_func style calls should produce
   *      consistent results aside from any exceptions mentioned in Zend PHP's
   *      documentation. Therefore, "B::foo()" and "call_user_func('B::foo')"
   *      and "call_user_func(array('B', 'foo'))" should all produce consistent
   *      results.
   *   4) call_user_func() and forward_static_call() must never forward the
   *      caller's late bound class to the callee if the late bound class is
   *      not the same or a descendent of the class enclosing the method.
   *      Zend's implementation of forward_static_call contains a check for
   *      this, implying intent to uphold this invariant. 
   */
  public function testFoo1() {
    echo "############# testFoo1 ##############\n";
    B::foo(); // B D D
    C::foo(); // C D D
    D::foo(); // D D D
    F::foo(); // F F    (Zend: F D D) (Rule 1)
    G::foo(); // G G    (Zend: G D D) (Rule 1)
    H::foo(); // H H    (Zend: H D D) (Rule 1)
    parent::foo(); // B D D
    self::foo();   // C D D
    static::foo(); // D D D
    echo "****************\n";
    call_user_func(array('B','foo')); // B D D
    call_user_func(array('C','foo')); // C D D
    call_user_func(array('D','foo')); // D D D
    call_user_func(array('F','foo')); // F F
    call_user_func(array('G','foo')); // G G
    call_user_func(array('H','foo')); // H H
    call_user_func(array('parent','foo')); // B D D
    call_user_func(array('self','foo'));   // C D D
    call_user_func(array('static','foo')); // D D D
    echo "****************\n";
    call_user_func('B::foo'); // B D D
    call_user_func('C::foo'); // C D D
    call_user_func('D::foo'); // D D D
    call_user_func('F::foo'); // F F
    call_user_func('G::foo'); // G G
    call_user_func('H::foo'); // H H
    call_user_func('parent::foo'); // B D D
    call_user_func('self::foo');   // C D D
    call_user_func('static::foo'); // D D D
    echo "****************\n";
    call_user_func(array('B','B::foo')); // B D D
    call_user_func(array('B','C::foo')); // warning
    call_user_func(array('B','D::foo')); // warning
    call_user_func(array('B','F::foo')); // warning
    call_user_func(array('B','G::foo')); // warning
    call_user_func(array('B','H::foo')); // warning
    call_user_func(array('B','parent::foo')); // A D D
    call_user_func(array('B','self::foo'));   // B D D
    call_user_func(array('B','static::foo')); // warning
    echo "****************\n";
    call_user_func(array('G','B::foo')); // warning
    call_user_func(array('G','C::foo')); // warning
    call_user_func(array('G','D::foo')); // warning
    call_user_func(array('G','F::foo')); // F F
    call_user_func(array('G','G::foo')); // G G
    call_user_func(array('G','H::foo')); // warning
    call_user_func(array('G','parent::foo')); // F F  (Zend: F D D) (Rule 1)
    call_user_func(array('G','self::foo'));   // G G  (Zend: G D D) (Rule 1)
    call_user_func(array('G','static::foo')); // warning
    echo "****************\n";
    $b = new B;
    call_user_func(array($b,'foo')); // B B B
    call_user_func(array($b,'B::foo')); // B B B
    call_user_func(array($b,'C::foo')); // warning
    call_user_func(array($b,'D::foo')); // warning
    call_user_func(array($b,'F::foo')); // warning
    call_user_func(array($b,'G::foo')); // warning
    call_user_func(array($b,'H::foo')); // warning
    call_user_func(array($b,'parent::foo')); // A B B
    call_user_func(array($b,'self::foo'));   // B B B
    call_user_func(array($b,'static::foo')); // warning
    echo "****************\n";
    $g = new G;
    call_user_func(array($g,'foo')); // G G G
    call_user_func(array($g,'B::foo')); // warning
    call_user_func(array($g,'C::foo')); // warning
    call_user_func(array($g,'D::foo')); // warning
    call_user_func(array($g,'F::foo')); // F G G
    call_user_func(array($g,'G::foo')); // G G G
    call_user_func(array($g,'H::foo')); // warning
    call_user_func(array($g,'parent::foo')); // F G G
    call_user_func(array($g,'self::foo'));   // G G G
    call_user_func(array($g,'static::foo')); // warning
    echo "****************\n";
  }
  public static function testFoo2() {
    echo "############# testFoo2 ##############\n";
    B::foo(); // B B
    C::foo(); // C C
    D::foo(); // D D
    F::foo(); // F F
    G::foo(); // G G
    H::foo(); // H H
    parent::foo(); // B D
    self::foo();   // C D
    static::foo(); // D D
    echo "****************\n";
    call_user_func(array('B','foo')); // B B
    call_user_func(array('C','foo')); // C C
    call_user_func(array('D','foo')); // D D
    call_user_func(array('F','foo')); // F F
    call_user_func(array('G','foo')); // G G
    call_user_func(array('H','foo')); // H H
    call_user_func(array('parent','foo')); // B D
    call_user_func(array('self','foo'));   // C D
    call_user_func(array('static','foo')); // D D
    echo "****************\n";
    call_user_func('B::foo'); // B B
    call_user_func('C::foo'); // C C
    call_user_func('D::foo'); // D D
    call_user_func('F::foo'); // F F
    call_user_func('G::foo'); // G G
    call_user_func('H::foo'); // H H
    call_user_func('parent::foo'); // B D
    call_user_func('self::foo');   // C D
    call_user_func('static::foo'); // D D
    echo "****************\n";
    call_user_func(array('B','B::foo')); // B B
    call_user_func(array('B','C::foo')); // warning
    call_user_func(array('B','D::foo')); // warning
    call_user_func(array('B','F::foo')); // warning
    call_user_func(array('B','G::foo')); // warning
    call_user_func(array('B','H::foo')); // warning
    call_user_func(array('B','parent::foo')); // A D
    call_user_func(array('B','self::foo'));   // B D
    call_user_func(array('B','static::foo')); // warning
    echo "****************\n";
    call_user_func(array('G','B::foo')); // warning
    call_user_func(array('G','C::foo')); // warning
    call_user_func(array('G','D::foo')); // warning
    call_user_func(array('G','F::foo')); // F F
    call_user_func(array('G','G::foo')); // G G
    call_user_func(array('G','H::foo')); // warning
    call_user_func(array('G','parent::foo')); // F F   (Zend: F D) (Rule 4)
    call_user_func(array('G','self::foo'));   // G G   (Zend: G D) (Rule 4)
    call_user_func(array('G','static::foo')); // warning
    echo "****************\n";
    $b = new B;
    call_user_func(array($b,'foo'));    // B B B
    call_user_func(array($b,'B::foo')); // B B B
    call_user_func(array($b,'C::foo')); // warning
    call_user_func(array($b,'D::foo')); // warning
    call_user_func(array($b,'F::foo')); // warning
    call_user_func(array($b,'G::foo')); // warning
    call_user_func(array($b,'H::foo')); // warning
    call_user_func(array($b,'parent::foo')); // A B B
    call_user_func(array($b,'self::foo'));   // B B B
    call_user_func(array($b,'static::foo')); // warning
    echo "****************\n";
    $g = new G;
    call_user_func(array($g,'foo'));    // G G G
    call_user_func(array($g,'B::foo')); // warning
    call_user_func(array($g,'C::foo')); // warning
    call_user_func(array($g,'D::foo')); // warning
    call_user_func(array($g,'F::foo')); // F G G
    call_user_func(array($g,'G::foo')); // G G G
    call_user_func(array($g,'H::foo')); // warning
    call_user_func(array($g,'parent::foo')); // F G G
    call_user_func(array($g,'self::foo'));   // G G G
    call_user_func(array($g,'static::foo')); // warning
    echo "****************\n";
  }
  public function testBar1() {
    echo "############# testBar1 ##############\n";
    B::bar(); // B B
    C::bar(); // C C
    D::bar(); // D D
    F::bar(); // F F
    G::bar(); // G G
    H::bar(); // H H
    parent::bar(); // B D
    self::bar();   // C D
    static::bar(); // D D
    echo "****************\n";
    call_user_func(array('B','bar')); // B B   (Zend: B D) (Rule 3)
    call_user_func(array('C','bar')); // C C   (Zend: C D) (Rule 3)
    call_user_func(array('D','bar')); // D D
    call_user_func(array('F','bar')); // F F
    call_user_func(array('G','bar')); // G G
    call_user_func(array('H','bar')); // H H
    call_user_func(array('parent','bar')); // B D
    call_user_func(array('self','bar'));   // C D
    call_user_func(array('static','bar')); // D D
    echo "****************\n";
    call_user_func('B::bar'); // B B   (Zend: B D) (Rule 3)
    call_user_func('C::bar'); // C C   (Zend: C D) (Rule 3)
    call_user_func('D::bar'); // D D
    call_user_func('F::bar'); // F F
    call_user_func('G::bar'); // G G
    call_user_func('H::bar'); // H H
    call_user_func('parent::bar'); // B D
    call_user_func('self::bar');   // C D
    call_user_func('static::bar'); // D D
    echo "****************\n";
    call_user_func(array('B','B::bar')); // B B   (Zend: B D) (Rule 3)
    call_user_func(array('B','C::bar')); // warning
    call_user_func(array('B','D::bar')); // warning
    call_user_func(array('B','F::bar')); // warning
    call_user_func(array('B','G::bar')); // warning
    call_user_func(array('B','H::bar')); // warning
    call_user_func(array('B','parent::bar')); // A D
    call_user_func(array('B','self::bar'));   // B D
    call_user_func(array('B','static::bar')); // warning
    echo "****************\n";
    call_user_func(array('G','B::bar')); // warning
    call_user_func(array('G','C::bar')); // warning
    call_user_func(array('G','D::bar')); // warning
    call_user_func(array('G','F::bar')); // F F
    call_user_func(array('G','G::bar')); // G G
    call_user_func(array('G','H::bar')); // warning
    call_user_func(array('G','parent::bar')); // F F   (Zend: F D) (Rule 4)
    call_user_func(array('G','self::bar'));   // G G   (Zend: G D) (Rule 4)
    call_user_func(array('G','static::bar')); // warning
    echo "****************\n";
    $b = new B;
    call_user_func(array($b,'bar'));    // B B
    call_user_func(array($b,'B::bar')); // B B
    call_user_func(array($b,'C::bar')); // warning
    call_user_func(array($b,'D::bar')); // warning
    call_user_func(array($b,'F::bar')); // warning
    call_user_func(array($b,'G::bar')); // warning
    call_user_func(array($b,'H::bar')); // warning
    call_user_func(array($b,'parent::bar')); // A B
    call_user_func(array($b,'self::bar'));   // B B
    call_user_func(array($b,'static::bar')); // warning
    echo "****************\n";
    $g = new G;
    call_user_func(array($g,'bar'));    // G G
    call_user_func(array($g,'B::bar')); // warning
    call_user_func(array($g,'C::bar')); // warning
    call_user_func(array($g,'D::bar')); // warning
    call_user_func(array($g,'F::bar')); // F G
    call_user_func(array($g,'G::bar')); // G G
    call_user_func(array($g,'H::bar')); // warning
    call_user_func(array($g,'parent::bar')); // F G
    call_user_func(array($g,'self::bar'));   // G G
    call_user_func(array($g,'static::bar')); // warning
    echo "****************\n";
  }
  public static function testBar2() {
    echo "############# testBar2 ##############\n";
    B::bar(); // B B
    C::bar(); // C C
    D::bar(); // D D
    F::bar(); // F F
    G::bar(); // G G
    H::bar(); // H H
    parent::bar(); // B D
    self::bar();   // C D
    static::bar(); // D D
    echo "****************\n";
    call_user_func(array('B','bar')); // B B
    call_user_func(array('C','bar')); // C C
    call_user_func(array('D','bar')); // D D
    call_user_func(array('F','bar')); // F F
    call_user_func(array('G','bar')); // G G
    call_user_func(array('H','bar')); // H H
    call_user_func(array('parent','bar')); // B D
    call_user_func(array('self','bar'));   // C D
    call_user_func(array('static','bar')); // D D
    echo "****************\n";
    call_user_func('B::bar'); // B B
    call_user_func('C::bar'); // C C
    call_user_func('D::bar'); // D D
    call_user_func('F::bar'); // F F
    call_user_func('G::bar'); // G G
    call_user_func('H::bar'); // H H
    call_user_func('parent::bar'); // B D
    call_user_func('self::bar');   // C D
    call_user_func('static::bar'); // D D
    echo "****************\n";
    call_user_func(array('B','B::bar')); // B B
    call_user_func(array('B','C::bar')); // warning
    call_user_func(array('B','D::bar')); // warning
    call_user_func(array('B','F::bar')); // warning
    call_user_func(array('B','G::bar')); // warning
    call_user_func(array('B','H::bar')); // warning
    call_user_func(array('B','parent::bar')); // A D
    call_user_func(array('B','self::bar'));   // B D
    call_user_func(array('B','static::bar')); // warning
    echo "****************\n";
    call_user_func(array('G','B::bar')); // warning
    call_user_func(array('G','C::bar')); // warning
    call_user_func(array('G','D::bar')); // warning
    call_user_func(array('G','F::bar')); // F F
    call_user_func(array('G','G::bar')); // G G
    call_user_func(array('G','H::bar')); // warning
    call_user_func(array('G','parent::bar')); // F F   (Zend: F D) (Rule 4)
    call_user_func(array('G','self::bar'));   // G G   (Zend: G D) (Rule 4)
    call_user_func(array('G','static::bar')); // warning
    echo "****************\n";
    $b = new B;
    call_user_func(array($b,'bar'));    // B B
    call_user_func(array($b,'B::bar')); // B B
    call_user_func(array($b,'C::bar')); // warning
    call_user_func(array($b,'D::bar')); // warning
    call_user_func(array($b,'F::bar')); // warning
    call_user_func(array($b,'G::bar')); // warning
    call_user_func(array($b,'H::bar')); // warning
    call_user_func(array($b,'parent::bar')); // A B
    call_user_func(array($b,'self::bar'));   // B B
    call_user_func(array($b,'static::bar')); // warning
    echo "****************\n";
    $g = new G;
    call_user_func(array($g,'bar'));    // G G
    call_user_func(array($g,'B::bar')); // warning
    call_user_func(array($g,'C::bar')); // warning
    call_user_func(array($g,'D::bar')); // warning
    call_user_func(array($g,'F::bar')); // F G
    call_user_func(array($g,'G::bar')); // G G
    call_user_func(array($g,'H::bar')); // warning
    call_user_func(array($g,'parent::bar')); // F G
    call_user_func(array($g,'self::bar'));   // G G
    call_user_func(array($g,'static::bar')); // warning
    echo "****************\n";
  }
}

class D extends C {
  public function foo() {
    echo __CLASS__ . ' ' . get_called_class() . ' ' .
         (isset($this) ? get_class($this) : '') . "\n";
  }
  public static function bar() {
    echo __CLASS__ . ' ' . get_called_class() . ' ' .
         (isset($this) ? get_class($this) : '') . "\n";
  }
}

class F {
  public function foo() {
    echo __CLASS__ . ' ' . get_called_class() . ' ' .
         (isset($this) ? get_class($this) : '') . "\n";
  }
  public static function bar() {
    echo __CLASS__ . ' ' . get_called_class() . ' ' .
         (isset($this) ? get_class($this) : '') . "\n";
  }
}

class G extends F {
  public function foo() {
    echo __CLASS__ . ' ' . get_called_class() . ' ' .
         (isset($this) ? get_class($this) : '') . "\n";
  }
  public static function bar() {
    echo __CLASS__ . ' ' . get_called_class() . ' ' .
         (isset($this) ? get_class($this) : '') . "\n";
  }
  public function doFoo() {
    $this->foo();  // G G G
    B::foo();      // B B    (Zend: B G G) (Rule 1)
    C::foo();      // C C    (Zend: C G G) (Rule 1)
    D::foo();      // D D    (Zend: D G G) (Rule 1)
    F::foo();      // F G G
    G::foo();      // G G G
    H::foo();      // H H    (Zend: H G G) (Rule 1)
    parent::foo(); // F G G
    self::foo();   // G G G
    static::foo(); // G G G
    echo "****************\n";
  }
  public function doBar() {
    $this->bar();  // G G
    B::bar();      // B B
    C::bar();      // C C
    D::bar();      // D D
    F::bar();      // F F
    G::bar();      // G G
    H::bar();      // H H
    parent::bar(); // F G
    self::bar();   // G G
    static::bar(); // G G
    echo "****************\n";
  }
}

class H extends G {
  public function foo() {
    echo __CLASS__ . ' ' . get_called_class() . ' ' .
         (isset($this) ? get_class($this) : '') . "\n";
  }
  public static function bar() {
    echo __CLASS__ . ' ' . get_called_class() . ' ' .
         (isset($this) ? get_class($this) : '') . "\n";
  }
}

$d = new D;
$d->testFoo1();
D::testFoo2();
$d->testBar1();
D::testBar2();

$b = new B;
$g = new G;
echo "############# doFoo ##############\n";
$b->doFoo();
$g->doFoo();
echo "############# doBar ##############\n";
$b->doBar();
$g->doBar();


