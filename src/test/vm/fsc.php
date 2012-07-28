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
    forward_static_call(array('B','foo')); // B D D
    forward_static_call(array('C','foo')); // C D D
    forward_static_call(array('D','foo')); // D D D
    forward_static_call(array('F','foo')); // F F
    forward_static_call(array('G','foo')); // G G
    forward_static_call(array('H','foo')); // H H
    forward_static_call(array('parent','foo')); // B D D
    forward_static_call(array('self','foo'));   // C D D
    forward_static_call(array('static','foo')); // D D D
    echo "****************\n";
    forward_static_call('B::foo'); // B D D
    forward_static_call('C::foo'); // C D D
    forward_static_call('D::foo'); // D D D
    forward_static_call('F::foo'); // F F
    forward_static_call('G::foo'); // G G
    forward_static_call('H::foo'); // H H
    forward_static_call('parent::foo'); // B D D
    forward_static_call('self::foo');   // C D D
    forward_static_call('static::foo'); // D D D
    echo "****************\n";
    forward_static_call(array('B','B::foo')); // B D D
    forward_static_call(array('B','C::foo')); // warning
    forward_static_call(array('B','D::foo')); // warning
    forward_static_call(array('B','F::foo')); // warning
    forward_static_call(array('B','G::foo')); // warning
    forward_static_call(array('B','H::foo')); // warning
    forward_static_call(array('B','parent::foo')); // A D D
    forward_static_call(array('B','self::foo'));   // B D D
    forward_static_call(array('B','static::foo')); // warning
    echo "****************\n";
    forward_static_call(array('G','B::foo')); // warning
    forward_static_call(array('G','C::foo')); // warning
    forward_static_call(array('G','D::foo')); // warning
    forward_static_call(array('G','F::foo')); // F F
    forward_static_call(array('G','G::foo')); // G G
    forward_static_call(array('G','H::foo')); // warning
    forward_static_call(array('G','parent::foo')); // F F (Zend: F D D) (Rule 1)
    forward_static_call(array('G','self::foo'));   // G G (Zend: G D D) (Rule 1)
    forward_static_call(array('G','static::foo')); // warning
    echo "****************\n";
    $b = new B;
    forward_static_call(array($b,'foo'));    // B B B  (Zend: B D B) (Rule 2)
    forward_static_call(array($b,'B::foo')); // B B B  (Zend: B D B) (Rule 2)
    forward_static_call(array($b,'C::foo')); // warning
    forward_static_call(array($b,'D::foo')); // warning
    forward_static_call(array($b,'F::foo')); // warning
    forward_static_call(array($b,'G::foo')); // warning
    forward_static_call(array($b,'H::foo')); // warning
    forward_static_call(array($b,'parent::foo')); // A B B (Zend: A D B) (Rule2)
    forward_static_call(array($b,'self::foo'));   // B B B (Zend: B D B) (Rule2)
    forward_static_call(array($b,'static::foo')); // warning
    echo "****************\n";
    $g = new G;
    forward_static_call(array($g,'foo'));    // G G G
    forward_static_call(array($g,'B::foo')); // warning
    forward_static_call(array($g,'C::foo')); // warning
    forward_static_call(array($g,'D::foo')); // warning
    forward_static_call(array($g,'F::foo')); // F G G
    forward_static_call(array($g,'G::foo')); // G G G
    forward_static_call(array($g,'H::foo')); // warning
    forward_static_call(array($g,'parent::foo')); // F G G
    forward_static_call(array($g,'self::foo'));   // G G G
    forward_static_call(array($g,'static::foo')); // warning
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
    forward_static_call(array('B','foo')); // B D
    forward_static_call(array('C','foo')); // C D
    forward_static_call(array('D','foo')); // D D
    forward_static_call(array('F','foo')); // F F
    forward_static_call(array('G','foo')); // G G
    forward_static_call(array('H','foo')); // H H
    forward_static_call(array('parent','foo')); // B D
    forward_static_call(array('self','foo'));   // C D
    forward_static_call(array('static','foo')); // D D
    echo "****************\n";
    forward_static_call('B::foo'); // B D
    forward_static_call('C::foo'); // C D
    forward_static_call('D::foo'); // D D
    forward_static_call('F::foo'); // F F
    forward_static_call('G::foo'); // G G
    forward_static_call('H::foo'); // H H
    forward_static_call('parent::foo'); // B D
    forward_static_call('self::foo');   // C D
    forward_static_call('static::foo'); // D D
    echo "****************\n";
    forward_static_call(array('B','B::foo')); // B D
    forward_static_call(array('B','C::foo')); // warning
    forward_static_call(array('B','D::foo')); // warning
    forward_static_call(array('B','F::foo')); // warning
    forward_static_call(array('B','G::foo')); // warning
    forward_static_call(array('B','H::foo')); // warning
    forward_static_call(array('B','parent::foo')); // A D
    forward_static_call(array('B','self::foo'));   // B D
    forward_static_call(array('B','static::foo')); // warning
    echo "****************\n";
    forward_static_call(array('G','B::foo')); // warning
    forward_static_call(array('G','C::foo')); // warning
    forward_static_call(array('G','D::foo')); // warning
    forward_static_call(array('G','F::foo')); // F F
    forward_static_call(array('G','G::foo')); // G G
    forward_static_call(array('G','H::foo')); // warning
    forward_static_call(array('G','parent::foo')); // F F  (Zend: F D) (Rule 4)
    forward_static_call(array('G','self::foo'));   // G G  (Zend: G D) (Rule 4)
    forward_static_call(array('G','static::foo'));
    echo "****************\n";
    $b = new B;
    forward_static_call(array($b,'foo'));    // B B B
    forward_static_call(array($b,'B::foo')); // B B B
    forward_static_call(array($b,'C::foo')); // warning
    forward_static_call(array($b,'D::foo')); // warning
    forward_static_call(array($b,'F::foo')); // warning
    forward_static_call(array($b,'G::foo')); // warning
    forward_static_call(array($b,'H::foo')); // warning
    forward_static_call(array($b,'parent::foo')); // A B B
    forward_static_call(array($b,'self::foo'));   // B B B
    forward_static_call(array($b,'static::foo')); // warning
    echo "****************\n";
    $g = new G;
    forward_static_call(array($g,'foo'));    // G G G
    forward_static_call(array($g,'B::foo')); // warning
    forward_static_call(array($g,'C::foo')); // warning
    forward_static_call(array($g,'D::foo')); // warning
    forward_static_call(array($g,'F::foo')); // F G G
    forward_static_call(array($g,'G::foo')); // G G G
    forward_static_call(array($g,'H::foo')); // warning
    forward_static_call(array($g,'parent::foo')); // F G G
    forward_static_call(array($g,'self::foo'));   // G G G
    forward_static_call(array($g,'static::foo')); // warning
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
    forward_static_call(array('B','bar')); // B D
    forward_static_call(array('C','bar')); // C D
    forward_static_call(array('D','bar')); // D D
    forward_static_call(array('F','bar')); // F F
    forward_static_call(array('G','bar')); // G G
    forward_static_call(array('H','bar')); // H H
    forward_static_call(array('parent','bar')); // B D
    forward_static_call(array('self','bar'));   // C D
    forward_static_call(array('static','bar')); // D D
    echo "****************\n";
    forward_static_call('B::bar'); // B D
    forward_static_call('C::bar'); // C D
    forward_static_call('D::bar'); // D D
    forward_static_call('F::bar'); // F F
    forward_static_call('G::bar'); // G G
    forward_static_call('H::bar'); // H H
    forward_static_call('parent::bar'); // B D
    forward_static_call('self::bar');   // C D
    forward_static_call('static::bar'); // D D
    echo "****************\n";
    forward_static_call(array('B','B::bar')); // B D
    forward_static_call(array('B','C::bar')); // warning
    forward_static_call(array('B','D::bar')); // warning
    forward_static_call(array('B','F::bar')); // warning
    forward_static_call(array('B','G::bar')); // warning
    forward_static_call(array('B','H::bar')); // warning
    forward_static_call(array('B','parent::bar')); // A D
    forward_static_call(array('B','self::bar'));   // B D
    forward_static_call(array('B','static::bar')); // warning
    echo "****************\n";
    forward_static_call(array('G','B::bar'));
    forward_static_call(array('G','C::bar'));
    forward_static_call(array('G','D::bar'));
    forward_static_call(array('G','F::bar')); // F F
    forward_static_call(array('G','G::bar')); // G G
    forward_static_call(array('G','H::bar'));
    forward_static_call(array('G','parent::bar')); // F F  (Zend: F D) (Rule 4)
    forward_static_call(array('G','self::bar'));   // G G  (Zend: G D) (Rule 4)
    forward_static_call(array('G','static::bar'));
    echo "****************\n";
    $b = new B;
    forward_static_call(array($b,'bar'));    // B D
    forward_static_call(array($b,'B::bar')); // B D
    forward_static_call(array($b,'C::bar')); // warning
    forward_static_call(array($b,'D::bar')); // warning
    forward_static_call(array($b,'F::bar')); // warning
    forward_static_call(array($b,'G::bar')); // warning
    forward_static_call(array($b,'H::bar')); // warning
    forward_static_call(array($b,'parent::bar')); // A D
    forward_static_call(array($b,'self::bar'));   // B D
    forward_static_call(array($b,'static::bar')); // warning
    echo "****************\n";
    $g = new G;
    forward_static_call(array($g,'bar'));    // G G
    forward_static_call(array($g,'B::bar')); // warning
    forward_static_call(array($g,'C::bar')); // warning
    forward_static_call(array($g,'D::bar')); // warning
    forward_static_call(array($g,'F::bar')); // F G
    forward_static_call(array($g,'G::bar')); // G G
    forward_static_call(array($g,'H::bar')); // warning
    forward_static_call(array($g,'parent::bar')); // F G
    forward_static_call(array($g,'self::bar'));   // G G
    forward_static_call(array($g,'static::bar')); // warning
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
    forward_static_call(array('B','bar')); // B D
    forward_static_call(array('C','bar')); // C D
    forward_static_call(array('D','bar')); // D D
    forward_static_call(array('F','bar')); // F F
    forward_static_call(array('G','bar')); // G G
    forward_static_call(array('H','bar')); // H H
    forward_static_call(array('parent','bar')); // B D
    forward_static_call(array('self','bar'));   // C D
    forward_static_call(array('static','bar')); // D D
    echo "****************\n";
    forward_static_call('B::bar'); // B D
    forward_static_call('C::bar'); // C D
    forward_static_call('D::bar'); // D D
    forward_static_call('F::bar'); // F F
    forward_static_call('G::bar'); // G G
    forward_static_call('H::bar'); // H H
    forward_static_call('parent::bar'); // B D
    forward_static_call('self::bar');   // C D
    forward_static_call('static::bar'); // D D
    echo "****************\n";
    forward_static_call(array('B','B::bar')); // B D
    forward_static_call(array('B','C::bar')); // warning
    forward_static_call(array('B','D::bar')); // warning
    forward_static_call(array('B','F::bar')); // warning
    forward_static_call(array('B','G::bar')); // warning
    forward_static_call(array('B','H::bar')); // warning
    forward_static_call(array('B','parent::bar')); // A D
    forward_static_call(array('B','self::bar'));   // B D
    forward_static_call(array('B','static::bar')); // warning
    echo "****************\n";
    forward_static_call(array('G','B::bar'));
    forward_static_call(array('G','C::bar'));
    forward_static_call(array('G','D::bar'));
    forward_static_call(array('G','F::bar')); // F F
    forward_static_call(array('G','G::bar')); // G G
    forward_static_call(array('G','H::bar'));
    forward_static_call(array('G','parent::bar')); // F F (Zend: F D) (Rule 4)
    forward_static_call(array('G','self::bar'));   // G G (Zend: G D) (Rule 4)
    forward_static_call(array('G','static::bar'));
    echo "****************\n";
    $b = new B;
    forward_static_call(array($b,'bar'));    // B D
    forward_static_call(array($b,'B::bar')); // B D
    forward_static_call(array($b,'C::bar'));
    forward_static_call(array($b,'D::bar'));
    forward_static_call(array($b,'F::bar'));
    forward_static_call(array($b,'G::bar'));
    forward_static_call(array($b,'H::bar'));
    forward_static_call(array($b,'parent::bar')); // A D
    forward_static_call(array($b,'self::bar'));   // B D
    forward_static_call(array($b,'static::bar'));
    echo "****************\n";
    $g = new G;
    forward_static_call(array($g,'bar'));    // G G
    forward_static_call(array($g,'B::bar')); // warning
    forward_static_call(array($g,'C::bar')); // warning
    forward_static_call(array($g,'D::bar')); // warning
    forward_static_call(array($g,'F::bar')); // F G
    forward_static_call(array($g,'G::bar')); // G G
    forward_static_call(array($g,'H::bar')); // warning
    forward_static_call(array($g,'parent::bar')); // F G
    forward_static_call(array($g,'self::bar'));   // G G
    forward_static_call(array($g,'static::bar')); // warning
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


