<?php
error_reporting(-1);

// FIXME: This test enables rename function, which has an impact on how
// function dispatch and entry are treated. Without this option, behavior
// differs between jit and non-jit code.

class A {
  public function meth() {
    echo __CLASS__ . ' ' . get_called_class() .
         (isset($this) ? ' '.get_class($this) : '') . "\n";
  }
  public static function staticMeth() {
    echo __CLASS__ . ' ' . get_called_class() .
         (isset($this) ? ' '.get_class($this) : '') . "\n";
  }
}

class B extends A {
  public function meth() {
    echo __CLASS__ . ' ' . get_called_class() .
         (isset($this) ? ' '.get_class($this) : '') . "\n";
  }
  public static function staticMeth() {
    echo __CLASS__ . ' ' . get_called_class() .
         (isset($this) ? ' '.get_class($this) : '') . "\n";
  }
  public function doMeth() {
    $this->meth();  // B B B
    B::meth();      // B B B
    C::meth();      // C C   (Zend outputs: C B B) (Rule 1)
    D::meth();      // D D   (Zend outputs: D B B) (Rule 1)
    F::meth();      // F F   (Zend outputs: F B B) (Rule 1)
    G::meth();      // G G   (Zend outputs: G B B) (Rule 1)
    H::meth();      // H H   (Zend outputs: H B B) (Rule 1)
    parent::meth(); // A B B
    self::meth();   // B B B
    static::meth(); // B B B
    echo "****************\n";
  }
  public function doStaticMeth() {
    $this->staticMeth();  // B B
    B::staticMeth();      // B B
    C::staticMeth();      // C C
    D::staticMeth();      // D D
    F::staticMeth();      // F F
    G::staticMeth();      // G G
    H::staticMeth();      // H H
    parent::staticMeth(); // A B
    self::staticMeth();   // B B
    static::staticMeth(); // B B
    echo "****************\n";
  }
}

class C extends B {
  public function meth() {
    echo __CLASS__ . ' ' . get_called_class() .
         (isset($this) ? ' '.get_class($this) : '') . "\n";
  }
  public static function staticMeth() {
    echo __CLASS__ . ' ' . get_called_class() .
         (isset($this) ? ' '.get_class($this) : '') . "\n";
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
   *      documentation. Therefore, "B::meth()" and "call_user_func('B::meth')"
   *      and "call_user_func(array('B', 'meth'))" should all produce consistent
   *      results.
   *   4) call_user_func() and forward_static_call() must never forward the
   *      caller's late bound class to the callee if the late bound class is
   *      not the same or a descendent of the class enclosing the method.
   *      Zend's implementation of forward_static_call contains a check for
   *      this, implying intent to uphold this invariant.
   */
  public function testMeth1() {
    echo "############# testMeth1 ##############\n";
    B::meth(); // B D D
    C::meth(); // C D D
    D::meth(); // D D D
    F::meth(); // F F    (Zend: F D D) (Rule 1)
    G::meth(); // G G    (Zend: G D D) (Rule 1)
    H::meth(); // H H    (Zend: H D D) (Rule 1)
    parent::meth(); // B D D
    self::meth();   // C D D
    static::meth(); // D D D
    echo "****************\n";
    call_user_func('B::meth'); // B D D
    call_user_func('C::meth'); // C D D
    call_user_func('D::meth'); // D D D
    call_user_func('F::meth'); // F F
    call_user_func('G::meth'); // G G
    call_user_func('H::meth'); // H H
    call_user_func('parent::meth'); // B D D
    call_user_func('self::meth');   // C D D
    call_user_func('static::meth'); // D D D
    echo "****************\n";
    call_user_func(array('B','meth')); // B D D
    call_user_func(array('C','meth')); // C D D
    call_user_func(array('D','meth')); // D D D
    call_user_func(array('F','meth')); // F F
    call_user_func(array('G','meth')); // G G
    call_user_func(array('H','meth')); // H H
    call_user_func(array('parent','meth')); // B D D
    call_user_func(array('self','meth'));   // C D D
    call_user_func(array('static','meth')); // D D D
    echo "****************\n";
    call_user_func(array('B','B::meth')); // B D D
    call_user_func(array('B','C::meth')); // warning
    call_user_func(array('B','D::meth')); // warning
    call_user_func(array('B','F::meth')); // warning
    call_user_func(array('B','G::meth')); // warning
    call_user_func(array('B','H::meth')); // warning
    call_user_func(array('B','parent::meth')); // A D D
    call_user_func(array('B','self::meth'));   // B D D
    call_user_func(array('B','static::meth')); // warning
    echo "****************\n";
    $call = array('B','B::meth'); $call(); // B D D
    // $call = array('B','C::meth'); $call(); // expected fatal
    // $call = array('B','D::meth'); $call(); // expected fatal
    // $call = array('B','F::meth'); $call(); // expected fatal
    // $call = array('B','G::meth'); $call(); // expected fatal
    // $call = array('B','H::meth'); $call(); // expected fatal
    $call = array('B','parent::meth'); $call(); // A D D
    $call = array('B','self::meth'); $call();   // B D D
    // $call = array('B','static::meth'); $call(); // expected fatal
    echo "****************\n";
    call_user_func(array('G','B::meth')); // warning
    call_user_func(array('G','C::meth')); // warning
    call_user_func(array('G','D::meth')); // warning
    call_user_func(array('G','F::meth')); // F F
    call_user_func(array('G','G::meth')); // G G
    call_user_func(array('G','H::meth')); // warning
    call_user_func(array('G','parent::meth')); // F F  (Zend: F D D) (Rule 1)
    call_user_func(array('G','self::meth'));   // G G  (Zend: G D D) (Rule 1)
    call_user_func(array('G','static::meth')); // warning
    echo "****************\n";
    // $call = array('G','B::meth'); $call(); // expected fatal
    // $call = array('G','C::meth'); $call(); // expected fatal
    // $call = array('G','D::meth'); $call(); // expected fatal
    $call = array('G','F::meth'); $call(); // F F
    $call = array('G','G::meth'); $call(); // G G
    // $call = array('G','H::meth'); $call(); // expected fatal
    $call = array('G','parent::meth'); $call(); // F F  (Zend: F D D) (Rule 1)
    $call = array('G','self::meth'); $call();   // G G  (Zend: G D D) (Rule 1)
    // $call = array('G','static::meth'); $call(); // expected fatal
    echo "****************\n";
    $b = new B;
    call_user_func(array($b,'meth')); // B B B
    call_user_func(array($b,'B::meth')); // B B B
    call_user_func(array($b,'C::meth')); // warning
    call_user_func(array($b,'D::meth')); // warning
    call_user_func(array($b,'F::meth')); // warning
    call_user_func(array($b,'G::meth')); // warning
    call_user_func(array($b,'H::meth')); // warning
    call_user_func(array($b,'parent::meth')); // A B B
    call_user_func(array($b,'self::meth'));   // B B B
    call_user_func(array($b,'static::meth')); // warning
    echo "****************\n";
    $b = new B;
    $call = array($b,'meth'); $call(); // B B B
    $call = array($b,'B::meth'); $call(); // B B B
    // $call = array($b,'C::meth'); $call(); // expected fatal
    // $call = array($b,'D::meth'); $call(); // expected fatal
    // $call = array($b,'F::meth'); $call(); // expected fatal
    // $call = array($b,'G::meth'); $call(); // expected fatal
    // $call = array($b,'H::meth'); $call(); // expected fatal
    $call = array($b,'parent::meth'); $call(); // A B B
    $call = array($b,'self::meth'); $call();   // B B B
    // $call = array($b,'static::meth'); $call(); // expected fatal
    echo "****************\n";
    $g = new G;
    call_user_func(array($g,'meth')); // G G G
    call_user_func(array($g,'B::meth')); // warning
    call_user_func(array($g,'C::meth')); // warning
    call_user_func(array($g,'D::meth')); // warning
    call_user_func(array($g,'F::meth')); // F G G
    call_user_func(array($g,'G::meth')); // G G G
    call_user_func(array($g,'H::meth')); // warning
    call_user_func(array($g,'parent::meth')); // F G G
    call_user_func(array($g,'self::meth'));   // G G G
    call_user_func(array($g,'static::meth')); // warning
    echo "****************\n";
    $g = new G;
    $call = array($g,'meth'); $call(); // G G G
    // $call = array($g,'B::meth'); $call(); // expected fatal
    // $call = array($g,'C::meth'); $call(); // expected fatal
    // $call = array($g,'D::meth'); $call(); // expected fatal
    $call = array($g,'F::meth'); $call(); // F G G
    $call = array($g,'G::meth'); $call(); // G G G
    // $call = array($g,'H::meth'); $call(); // expected fatal
    $call = array($g,'parent::meth'); $call(); // F G G
    $call = array($g,'self::meth'); $call();   // G G G
    // $call = array($g,'static::meth'); $call(); // expected fatal
    echo "****************\n";
  }
  public static function testMeth2() {
    echo "############# testMeth2 ##############\n";
    B::meth(); // B B
    C::meth(); // C C
    D::meth(); // D D
    F::meth(); // F F
    G::meth(); // G G
    H::meth(); // H H
    parent::meth(); // B D
    self::meth();   // C D
    static::meth(); // D D
    echo "****************\n";
    call_user_func('B::meth'); // B B
    call_user_func('C::meth'); // C C
    call_user_func('D::meth'); // D D
    call_user_func('F::meth'); // F F
    call_user_func('G::meth'); // G G
    call_user_func('H::meth'); // H H
    call_user_func('parent::meth'); // B D
    call_user_func('self::meth');   // C D
    call_user_func('static::meth'); // D D
    echo "****************\n";
    call_user_func(array('B','meth')); // B B
    call_user_func(array('C','meth')); // C C
    call_user_func(array('D','meth')); // D D
    call_user_func(array('F','meth')); // F F
    call_user_func(array('G','meth')); // G G
    call_user_func(array('H','meth')); // H H
    call_user_func(array('parent','meth')); // B D
    call_user_func(array('self','meth'));   // C D
    call_user_func(array('static','meth')); // D D
    echo "****************\n";
    $call = array('B','meth'); $call(); // B B
    $call = array('C','meth'); $call(); // C C
    $call = array('D','meth'); $call(); // D D
    $call = array('F','meth'); $call(); // F F
    $call = array('G','meth'); $call(); // G G
    $call = array('H','meth'); $call(); // H H
    $call = array('parent','meth'); $call(); // B D
    $call = array('self','meth'); $call();   // C D
    $call = array('static','meth'); $call(); // D D
    echo "****************\n";
    call_user_func(array('B','B::meth')); // B B
    call_user_func(array('B','C::meth')); // warning
    call_user_func(array('B','D::meth')); // warning
    call_user_func(array('B','F::meth')); // warning
    call_user_func(array('B','G::meth')); // warning
    call_user_func(array('B','H::meth')); // warning
    call_user_func(array('B','parent::meth')); // A D
    call_user_func(array('B','self::meth'));   // B D
    call_user_func(array('B','static::meth')); // warning
    echo "****************\n";
    $call = array('B','B::meth'); $call(); // B B
    // $call = array('B','C::meth'); $call(); // expected fatal
    // $call = array('B','D::meth'); $call(); // expected fatal
    // $call = array('B','F::meth'); $call(); // expected fatal
    // $call = array('B','G::meth'); $call(); // expected fatal
    // $call = array('B','H::meth'); $call(); // expected fatal
    $call = array('B','parent::meth'); $call(); // A D
    $call = array('B','self::meth'); $call();   // B D
    // $call = array('B','static::meth'); $call(); // expected fatal
    echo "****************\n";
    call_user_func(array('G','B::meth')); // warning
    call_user_func(array('G','C::meth')); // warning
    call_user_func(array('G','D::meth')); // warning
    call_user_func(array('G','F::meth')); // F F
    call_user_func(array('G','G::meth')); // G G
    call_user_func(array('G','H::meth')); // warning
    call_user_func(array('G','parent::meth')); // F F   (Zend: F D) (Rule 4)
    call_user_func(array('G','self::meth'));   // G G   (Zend: G D) (Rule 4)
    call_user_func(array('G','static::meth')); // warning
    echo "****************\n";
    // $call = array('G','B::meth'); $call(); // warning
    // $call = array('G','C::meth'); $call(); // warning
    // $call = array('G','D::meth'); $call(); // warning
    $call = array('G','F::meth'); $call(); // F F
    $call = array('G','G::meth'); $call(); // G G
    // $call = array('G','H::meth'); $call(); // warning
    $call = array('G','parent::meth'); $call(); // F F   (Zend: F D) (Rule 4)
    $call = array('G','self::meth'); $call();   // G G   (Zend: G D) (Rule 4)
    // $call = array('G','static::meth'); $call(); // warning
    echo "****************\n";
    $b = new B;
    call_user_func(array($b,'meth'));    // B B B
    call_user_func(array($b,'B::meth')); // B B B
    call_user_func(array($b,'C::meth')); // warning
    call_user_func(array($b,'D::meth')); // warning
    call_user_func(array($b,'F::meth')); // warning
    call_user_func(array($b,'G::meth')); // warning
    call_user_func(array($b,'H::meth')); // warning
    call_user_func(array($b,'parent::meth')); // A B B
    call_user_func(array($b,'self::meth'));   // B B B
    call_user_func(array($b,'static::meth')); // warning
    echo "****************\n";
    $b = new B;
    $call = array($b,'meth'); $call();    // B B B
    $call = array($b,'B::meth'); $call(); // B B B
    // $call = array($b,'C::meth'); $call(); // expected fatal
    // $call = array($b,'D::meth'); $call(); // expected fatal
    // $call = array($b,'F::meth'); $call(); // expected fatal
    // $call = array($b,'G::meth'); $call(); // expected fatal
    // $call = array($b,'H::meth'); $call(); // expected fatal
    $call = array($b,'parent::meth'); $call(); // A B B
    $call = array($b,'self::meth'); $call();   // B B B
    // $call = array($b,'static::meth'); $call(); // expected fatal
    echo "****************\n";
    $g = new G;
    call_user_func(array($g,'meth'));    // G G G
    call_user_func(array($g,'B::meth')); // warning
    call_user_func(array($g,'C::meth')); // warning
    call_user_func(array($g,'D::meth')); // warning
    call_user_func(array($g,'F::meth')); // F G G
    call_user_func(array($g,'G::meth')); // G G G
    call_user_func(array($g,'H::meth')); // warning
    call_user_func(array($g,'parent::meth')); // F G G
    call_user_func(array($g,'self::meth'));   // G G G
    call_user_func(array($g,'static::meth')); // warning
    echo "****************\n";
    $g = new G;
    $call = array($g,'meth'); $call();    // G G G
    // $call = array($g,'B::meth'); $call(); // expected fatal
    // $call = array($g,'C::meth'); $call(); // expected fatal
    // $call = array($g,'D::meth'); $call(); // expected fatal
    $call = array($g,'F::meth'); $call(); // F G G
    $call = array($g,'G::meth'); $call(); // G G G
    // $call = array($g,'H::meth'); $call(); // expected fatal
    $call = array($g,'parent::meth'); $call(); // F G G
    $call = array($g,'self::meth'); $call();   // G G G
    // $call = array($g,'static::meth'); $call(); // expected fatal
    echo "****************\n";
  }
  public function testStaticMeth1() {
    echo "############# testStaticMeth1 ##############\n";
    B::staticMeth(); // B B
    C::staticMeth(); // C C
    D::staticMeth(); // D D
    F::staticMeth(); // F F
    G::staticMeth(); // G G
    H::staticMeth(); // H H
    parent::staticMeth(); // B D
    self::staticMeth();   // C D
    static::staticMeth(); // D D
    echo "****************\n";
    call_user_func('B::staticMeth'); // B B   (Zend: B D) (Rule 3)
    call_user_func('C::staticMeth'); // C C   (Zend: C D) (Rule 3)
    call_user_func('D::staticMeth'); // D D
    call_user_func('F::staticMeth'); // F F
    call_user_func('G::staticMeth'); // G G
    call_user_func('H::staticMeth'); // H H
    call_user_func('parent::staticMeth'); // B D
    call_user_func('self::staticMeth');   // C D
    call_user_func('static::staticMeth'); // D D
    echo "****************\n";
    call_user_func(array('B','staticMeth')); // B B   (Zend: B D) (Rule 3)
    call_user_func(array('C','staticMeth')); // C C   (Zend: C D) (Rule 3)
    call_user_func(array('D','staticMeth')); // D D
    call_user_func(array('F','staticMeth')); // F F
    call_user_func(array('G','staticMeth')); // G G
    call_user_func(array('H','staticMeth')); // H H
    call_user_func(array('parent','staticMeth')); // B D
    call_user_func(array('self','staticMeth'));   // C D
    call_user_func(array('static','staticMeth')); // D D
    echo "****************\n";
    $call = array('B','staticMeth'); $call(); // B B   (Zend: B D) (Rule 3)
    $call = array('C','staticMeth'); $call(); // C C   (Zend: C D) (Rule 3)
    $call = array('D','staticMeth'); $call(); // D D
    $call = array('F','staticMeth'); $call(); // F F
    $call = array('G','staticMeth'); $call(); // G G
    $call = array('H','staticMeth'); $call(); // H H
    $call = array('parent','staticMeth'); $call(); // B D
    $call = array('self','staticMeth'); $call();   // C D
    $call = array('static','staticMeth'); $call(); // D D
    echo "****************\n";
    call_user_func(array('B','B::staticMeth')); // B B   (Zend: B D) (Rule 3)
    call_user_func(array('B','C::staticMeth')); // warning
    call_user_func(array('B','D::staticMeth')); // warning
    call_user_func(array('B','F::staticMeth')); // warning
    call_user_func(array('B','G::staticMeth')); // warning
    call_user_func(array('B','H::staticMeth')); // warning
    call_user_func(array('B','parent::staticMeth')); // A D
    call_user_func(array('B','self::staticMeth'));   // B D
    call_user_func(array('B','static::staticMeth')); // warning
    echo "****************\n";
    $call = array('B','B::staticMeth'); $call(); // B B   (Zend: B D) (Rule 3)
    // $call = array('B','C::staticMeth'); $call(); // expected fatal
    // $call = array('B','D::staticMeth'); $call(); // expected fatal
    // $call = array('B','F::staticMeth'); $call(); // expected fatal
    // $call = array('B','G::staticMeth'); $call(); // expected fatal
    // $call = array('B','H::staticMeth'); $call(); // expected fatal
    $call = array('B','parent::staticMeth'); $call(); // A D
    $call = array('B','self::staticMeth'); $call();   // B D
    // $call = array('B','static::staticMeth'); $call(); // expected fatal
    echo "****************\n";
    call_user_func(array('G','B::staticMeth')); // warning
    call_user_func(array('G','C::staticMeth')); // warning
    call_user_func(array('G','D::staticMeth')); // warning
    call_user_func(array('G','F::staticMeth')); // F F
    call_user_func(array('G','G::staticMeth')); // G G
    call_user_func(array('G','H::staticMeth')); // warning
    call_user_func(array('G','parent::staticMeth')); // F F   (Zend: F D) (Rule 4)
    call_user_func(array('G','self::staticMeth'));   // G G   (Zend: G D) (Rule 4)
    call_user_func(array('G','static::staticMeth')); // warning
    echo "****************\n";
    // $call = array('G','B::staticMeth'); $call(); // expected fatal
    // $call = array('G','C::staticMeth'); $call(); // expected fatal
    // $call = array('G','D::staticMeth'); $call(); // expected fatal
    $call = array('G','F::staticMeth'); $call(); // F F
    $call = array('G','G::staticMeth'); $call(); // G G
    // $call = array('G','H::staticMeth'); $call(); // expected fatal
    $call = array('G','parent::staticMeth'); $call(); // F F   (Zend: F D) (Rule 4)
    $call = array('G','self::staticMeth'); $call();   // G G   (Zend: G D) (Rule 4)
    // $call = array('G','static::staticMeth'); $call(); // expected fatal
    echo "****************\n";
    $b = new B;
    call_user_func(array($b,'staticMeth'));    // B B
    call_user_func(array($b,'B::staticMeth')); // B B
    call_user_func(array($b,'C::staticMeth')); // warning
    call_user_func(array($b,'D::staticMeth')); // warning
    call_user_func(array($b,'F::staticMeth')); // warning
    call_user_func(array($b,'G::staticMeth')); // warning
    call_user_func(array($b,'H::staticMeth')); // warning
    call_user_func(array($b,'parent::staticMeth')); // A B
    call_user_func(array($b,'self::staticMeth'));   // B B
    call_user_func(array($b,'static::staticMeth')); // warning
    echo "****************\n";
    $b = new B;
    $call = array($b,'staticMeth'); $call();    // B B
    $call = array($b,'B::staticMeth'); $call(); // B B
    // $call = array($b,'C::staticMeth'); $call(); // expected fatal
    // $call = array($b,'D::staticMeth'); $call(); // expected fatal
    // $call = array($b,'F::staticMeth'); $call(); // expected fatal
    // $call = array($b,'G::staticMeth'); $call(); // expected fatal
    // $call = array($b,'H::staticMeth'); $call(); // expected fatal
    $call = array($b,'parent::staticMeth'); $call(); // A B
    $call = array($b,'self::staticMeth'); $call();   // B B
    // $call = array($b,'static::staticMeth'); $call(); // expected fatal
    echo "****************\n";
    $g = new G;
    call_user_func(array($g,'staticMeth'));    // G G
    call_user_func(array($g,'B::staticMeth')); // warning
    call_user_func(array($g,'C::staticMeth')); // warning
    call_user_func(array($g,'D::staticMeth')); // warning
    call_user_func(array($g,'F::staticMeth')); // F G
    call_user_func(array($g,'G::staticMeth')); // G G
    call_user_func(array($g,'H::staticMeth')); // warning
    call_user_func(array($g,'parent::staticMeth')); // F G
    call_user_func(array($g,'self::staticMeth'));   // G G
    call_user_func(array($g,'static::staticMeth')); // warning
    echo "****************\n";
    $g = new G;
    $call = array($g,'staticMeth'); $call();    // G G
    // $call = array($g,'B::staticMeth'); $call(); // expected fatal
    // $call = array($g,'C::staticMeth'); $call(); // expected fatal
    // $call = array($g,'D::staticMeth'); $call(); // expected fatal
    $call = array($g,'F::staticMeth'); $call(); // F G
    $call = array($g,'G::staticMeth'); $call(); // G G
    // $call = array($g,'H::staticMeth'); $call(); // expected fatal
    $call = array($g,'parent::staticMeth'); $call(); // F G
    $call = array($g,'self::staticMeth'); $call();   // G G
    // $call = array($g,'static::staticMeth'); $call(); // expected fatal
    echo "****************\n";
  }
  public static function testStaticMeth2() {
    echo "############# testStaticMeth2 ##############\n";
    B::staticMeth(); // B B
    C::staticMeth(); // C C
    D::staticMeth(); // D D
    F::staticMeth(); // F F
    G::staticMeth(); // G G
    H::staticMeth(); // H H
    parent::staticMeth(); // B D
    self::staticMeth();   // C D
    static::staticMeth(); // D D
    echo "****************\n";
    call_user_func('B::staticMeth'); // B B
    call_user_func('C::staticMeth'); // C C
    call_user_func('D::staticMeth'); // D D
    call_user_func('F::staticMeth'); // F F
    call_user_func('G::staticMeth'); // G G
    call_user_func('H::staticMeth'); // H H
    call_user_func('parent::staticMeth'); // B D
    call_user_func('self::staticMeth');   // C D
    call_user_func('static::staticMeth'); // D D
    echo "****************\n";
    call_user_func(array('B','staticMeth')); // B B
    call_user_func(array('C','staticMeth')); // C C
    call_user_func(array('D','staticMeth')); // D D
    call_user_func(array('F','staticMeth')); // F F
    call_user_func(array('G','staticMeth')); // G G
    call_user_func(array('H','staticMeth')); // H H
    call_user_func(array('parent','staticMeth')); // B D
    call_user_func(array('self','staticMeth'));   // C D
    call_user_func(array('static','staticMeth')); // D D
    echo "****************\n";
    $call = array('B','staticMeth'); $call(); // B B
    $call = array('C','staticMeth'); $call(); // C C
    $call = array('D','staticMeth'); $call(); // D D
    $call = array('F','staticMeth'); $call(); // F F
    $call = array('G','staticMeth'); $call(); // G G
    $call = array('H','staticMeth'); $call(); // H H
    $call = array('parent','staticMeth'); $call(); // B D
    $call = array('self','staticMeth'); $call();   // C D
    $call = array('static','staticMeth'); $call(); // D D
    echo "****************\n";
    call_user_func(array('B','B::staticMeth')); // B B
    call_user_func(array('B','C::staticMeth')); // warning
    call_user_func(array('B','D::staticMeth')); // warning
    call_user_func(array('B','F::staticMeth')); // warning
    call_user_func(array('B','G::staticMeth')); // warning
    call_user_func(array('B','H::staticMeth')); // warning
    call_user_func(array('B','parent::staticMeth')); // A D
    call_user_func(array('B','self::staticMeth'));   // B D
    call_user_func(array('B','static::staticMeth')); // warning
    echo "****************\n";
    $call = array('B','B::staticMeth'); $call(); // B B
    // $call = array('B','C::staticMeth'); $call(); // expected fatal
    // $call = array('B','D::staticMeth'); $call(); // expected fatal
    // $call = array('B','F::staticMeth'); $call(); // expected fatal
    // $call = array('B','G::staticMeth'); $call(); // expected fatal
    // $call = array('B','H::staticMeth'); $call(); // expected fatal
    $call = array('B','parent::staticMeth'); $call(); // A D
    $call = array('B','self::staticMeth'); $call();   // B D
    // $call = array('B','static::staticMeth'); $call(); // expected fatal
    echo "****************\n";
    call_user_func(array('G','B::staticMeth')); // warning
    call_user_func(array('G','C::staticMeth')); // warning
    call_user_func(array('G','D::staticMeth')); // warning
    call_user_func(array('G','F::staticMeth')); // F F
    call_user_func(array('G','G::staticMeth')); // G G
    call_user_func(array('G','H::staticMeth')); // warning
    call_user_func(array('G','parent::staticMeth')); // F F   (Zend: F D) (Rule 4)
    call_user_func(array('G','self::staticMeth'));   // G G   (Zend: G D) (Rule 4)
    call_user_func(array('G','static::staticMeth')); // warning
    echo "****************\n";
    // $call = array('G','B::staticMeth'); $call(); // expected fatal
    // $call = array('G','C::staticMeth'); $call(); // expected fatal
    // $call = array('G','D::staticMeth'); $call(); // expected fatal
    $call = array('G','F::staticMeth'); $call(); // F F
    $call = array('G','G::staticMeth'); $call(); // G G
    // $call = array('G','H::staticMeth'); $call(); // expected fatal
    $call = array('G','parent::staticMeth'); $call(); // F F   (Zend: F D) (Rule 4)
    $call = array('G','self::staticMeth'); $call();   // G G   (Zend: G D) (Rule 4)
    // $call = array('G','static::staticMeth'); $call(); // expected fatal
    echo "****************\n";
    $b = new B;
    call_user_func(array($b,'staticMeth'));    // B B
    call_user_func(array($b,'B::staticMeth')); // B B
    call_user_func(array($b,'C::staticMeth')); // warning
    call_user_func(array($b,'D::staticMeth')); // warning
    call_user_func(array($b,'F::staticMeth')); // warning
    call_user_func(array($b,'G::staticMeth')); // warning
    call_user_func(array($b,'H::staticMeth')); // warning
    call_user_func(array($b,'parent::staticMeth')); // A B
    call_user_func(array($b,'self::staticMeth'));   // B B
    call_user_func(array($b,'static::staticMeth')); // warning
    echo "****************\n";
    $b = new B;
    $call = array($b,'staticMeth'); $call();    // B B
    $call = array($b,'B::staticMeth'); $call(); // B B
    // $call = array($b,'C::staticMeth'); $call(); // expected fatal
    // $call = array($b,'D::staticMeth'); $call(); // expected fatal
    // $call = array($b,'F::staticMeth'); $call(); // expected fatal
    // $call = array($b,'G::staticMeth'); $call(); // expected fatal
    // $call = array($b,'H::staticMeth'); $call(); // expected fatal
    $call = array($b,'parent::staticMeth'); $call(); // A B
    $call = array($b,'self::staticMeth'); $call();   // B B
    // $call = array($b,'static::staticMeth'); $call(); // expected fatal
    echo "****************\n";
    $g = new G;
    call_user_func(array($g,'staticMeth'));    // G G
    call_user_func(array($g,'B::staticMeth')); // warning
    call_user_func(array($g,'C::staticMeth')); // warning
    call_user_func(array($g,'D::staticMeth')); // warning
    call_user_func(array($g,'F::staticMeth')); // F G
    call_user_func(array($g,'G::staticMeth')); // G G
    call_user_func(array($g,'H::staticMeth')); // warning
    call_user_func(array($g,'parent::staticMeth')); // F G
    call_user_func(array($g,'self::staticMeth'));   // G G
    call_user_func(array($g,'static::staticMeth')); // warning
    echo "****************\n";
    $g = new G;
    $call = array($g,'staticMeth'); $call();    // G G
    // $call = array($g,'B::staticMeth'); $call(); // warning
    // $call = array($g,'C::staticMeth'); $call(); // warning
    // $call = array($g,'D::staticMeth'); $call(); // warning
    $call = array($g,'F::staticMeth'); $call(); // F G
    $call = array($g,'G::staticMeth'); $call(); // G G
    // $call = array($g,'H::staticMeth'); $call(); // warning
    $call = array($g,'parent::staticMeth'); $call(); // F G
    $call = array($g,'self::staticMeth'); $call();   // G G
    // $call = array($g,'static::staticMeth'); $call(); // warning
    echo "****************\n";
  }
}

class D extends C {
  public function meth() {
    echo __CLASS__ . ' ' . get_called_class() .
         (isset($this) ? ' '.get_class($this) : '') . "\n";
  }
  public static function staticMeth() {
    echo __CLASS__ . ' ' . get_called_class() .
         (isset($this) ? ' '.get_class($this) : '') . "\n";
  }
}

class F {
  public function meth() {
    echo __CLASS__ . ' ' . get_called_class() .
         (isset($this) ? ' '.get_class($this) : '') . "\n";
  }
  public static function staticMeth() {
    echo __CLASS__ . ' ' . get_called_class() .
         (isset($this) ? ' '.get_class($this) : '') . "\n";
  }
}

class G extends F {
  public function meth() {
    echo __CLASS__ . ' ' . get_called_class() .
         (isset($this) ? ' '.get_class($this) : '') . "\n";
  }
  public static function staticMeth() {
    echo __CLASS__ . ' ' . get_called_class() .
         (isset($this) ? ' '.get_class($this) : '') . "\n";
  }
  public function doMeth() {
    $this->meth();  // G G G
    B::meth();      // B B    (Zend: B G G) (Rule 1)
    C::meth();      // C C    (Zend: C G G) (Rule 1)
    D::meth();      // D D    (Zend: D G G) (Rule 1)
    F::meth();      // F G G
    G::meth();      // G G G
    H::meth();      // H H    (Zend: H G G) (Rule 1)
    parent::meth(); // F G G
    self::meth();   // G G G
    static::meth(); // G G G
    echo "****************\n";
  }
  public function doStaticMeth() {
    $this->staticMeth();  // G G
    B::staticMeth();      // B B
    C::staticMeth();      // C C
    D::staticMeth();      // D D
    F::staticMeth();      // F F
    G::staticMeth();      // G G
    H::staticMeth();      // H H
    parent::staticMeth(); // F G
    self::staticMeth();   // G G
    static::staticMeth(); // G G
    echo "****************\n";
  }
}

class H extends G {
  public function meth() {
    echo __CLASS__ . ' ' . get_called_class() .
         (isset($this) ? ' '.get_class($this) : '') . "\n";
  }
  public static function staticMeth() {
    echo __CLASS__ . ' ' . get_called_class() .
         (isset($this) ? ' '.get_class($this) : '') . "\n";
  }
}


function main() {
  $d = new D;
  $d->testMeth1();
  D::testMeth2();
  $d->testStaticMeth1();
  D::testStaticMeth2();

  $b = new B;
  $g = new G;
  echo "############# doMeth ##############\n";
  $b->doMeth();
  $g->doMeth();
  echo "############# doStaticMeth ##############\n";
  $b->doStaticMeth();
  $g->doStaticMeth();
}
main();
