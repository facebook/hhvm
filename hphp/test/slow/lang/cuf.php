<?hh

class A {
  public function meth() {
    echo __CLASS__ . ' ' . static::class .
         (isset($this) ? ' '.get_class($this) : '') . "\n";
  }
  public static function staticMeth() {
    echo __CLASS__ . ' ' . static::class . "\n";
  }
}

class B extends A {
  public function meth() {
    echo __CLASS__ . ' ' . static::class .
         (isset($this) ? ' '.get_class($this) : '') . "\n";
  }
  public static function staticMeth() {
    echo __CLASS__ . ' ' . static::class . "\n";
  }
  public function doMeth() {
    $this->meth();  // B B B
    B::meth();      // B B B





    parent::meth(); // A B B
    self::meth();   // B B B
    static::meth(); // B B B
    echo "****************\n";
  }
  public function doStaticMeth() {

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
    echo __CLASS__ . ' ' . static::class .
         (isset($this) ? ' '.get_class($this) : '') . "\n";
  }
  public static function staticMeth() {
    echo __CLASS__ . ' ' . static::class . "\n";
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
   *      "called class"). This invariant is stated explicitly in PHP5's
   *      documentation: "In non-static contexts, the called class will be the
   *      class of the object instance."
   *   3) Normal style calls and call_user_func style calls should produce
   *      consistent results aside from any exceptions mentioned in PHP5's
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



    parent::meth(); // B D D
    self::meth();   // C D D
    static::meth(); // D D D
    echo "****************\n";









    echo "****************\n";









    echo "****************\n";

    call_user_func(varray['B','C::meth']); // warning
    call_user_func(varray['B','D::meth']); // warning
    call_user_func(varray['B','F::meth']); // warning
    call_user_func(varray['B','G::meth']); // warning
    call_user_func(varray['B','H::meth']); // warning



    echo "****************\n";
    $call = varray['B','B::meth']; $call(); // B D D
    // $call = array('B','C::meth'); $call(); // expected fatal
    // $call = array('B','D::meth'); $call(); // expected fatal
    // $call = array('B','F::meth'); $call(); // expected fatal
    // $call = array('B','G::meth'); $call(); // expected fatal
    // $call = array('B','H::meth'); $call(); // expected fatal
    $call = varray['B','parent::meth']; $call(); // A D D
    $call = varray['B','self::meth']; $call();   // B D D
    // $call = array('B','static::meth'); $call(); // expected fatal
    echo "****************\n";
    call_user_func(varray['G','B::meth']); // warning
    call_user_func(varray['G','C::meth']); // warning
    call_user_func(varray['G','D::meth']); // warning


    call_user_func(varray['G','H::meth']); // warning



    echo "****************\n";
    // $call = array('G','B::meth'); $call(); // expected fatal
    // $call = array('G','C::meth'); $call(); // expected fatal
    // $call = array('G','D::meth'); $call(); // expected fatal
    // $call = array('G','F::meth'); $call(); // expected fatal
    // $call = array('G','G::meth'); $call(); // expected fatal
    // $call = array('G','H::meth'); $call(); // expected fatal
    // $call = array('G','parent::meth'); $call(); // expected fatal
    // $call = array('G','self::meth'); $call();   // expected fatal
    // $call = array('G','static::meth'); $call(); // expected fatal
    echo "****************\n";
    $b = new B;
    call_user_func(varray[$b,'meth']); // B B B
    call_user_func(varray[$b,'B::meth']); // B B B
    call_user_func(varray[$b,'C::meth']); // warning
    call_user_func(varray[$b,'D::meth']); // warning
    call_user_func(varray[$b,'F::meth']); // warning
    call_user_func(varray[$b,'G::meth']); // warning
    call_user_func(varray[$b,'H::meth']); // warning
    call_user_func(varray[$b,'parent::meth']); // A B B
    call_user_func(varray[$b,'self::meth']);   // B B B
    call_user_func(varray[$b,'static::meth']); // warning
    echo "****************\n";
    $b = new B;
    $call = varray[$b,'meth']; $call(); // B B B
    $call = varray[$b,'B::meth']; $call(); // B B B
    // $call = array($b,'C::meth'); $call(); // expected fatal
    // $call = array($b,'D::meth'); $call(); // expected fatal
    // $call = array($b,'F::meth'); $call(); // expected fatal
    // $call = array($b,'G::meth'); $call(); // expected fatal
    // $call = array($b,'H::meth'); $call(); // expected fatal
    $call = varray[$b,'parent::meth']; $call(); // A B B
    $call = varray[$b,'self::meth']; $call();   // B B B
    // $call = array($b,'static::meth'); $call(); // expected fatal
    echo "****************\n";
    $g = new G;
    call_user_func(varray[$g,'meth']); // G G G
    call_user_func(varray[$g,'B::meth']); // warning
    call_user_func(varray[$g,'C::meth']); // warning
    call_user_func(varray[$g,'D::meth']); // warning
    call_user_func(varray[$g,'F::meth']); // F G G
    call_user_func(varray[$g,'G::meth']); // G G G
    call_user_func(varray[$g,'H::meth']); // warning
    call_user_func(varray[$g,'parent::meth']); // F G G
    call_user_func(varray[$g,'self::meth']);   // G G G
    call_user_func(varray[$g,'static::meth']); // warning
    echo "****************\n";
    $g = new G;
    $call = varray[$g,'meth']; $call(); // G G G
    // $call = array($g,'B::meth'); $call(); // expected fatal
    // $call = array($g,'C::meth'); $call(); // expected fatal
    // $call = array($g,'D::meth'); $call(); // expected fatal
    $call = varray[$g,'F::meth']; $call(); // F G G
    $call = varray[$g,'G::meth']; $call(); // G G G
    // $call = array($g,'H::meth'); $call(); // expected fatal
    $call = varray[$g,'parent::meth']; $call(); // F G G
    $call = varray[$g,'self::meth']; $call();   // G G G
    // $call = array($g,'static::meth'); $call(); // expected fatal
    echo "****************\n";
  }
  public static function testMeth2() {
    echo "############# testMeth2 ##############\n";









    echo "****************\n";









    echo "****************\n";









    echo "****************\n";
    // $call = array('B','meth'); $call(); // expected fatal
    // $call = array('C','meth'); $call(); // expected fatal
    // $call = array('D','meth'); $call(); // expected fatal
    // $call = array('F','meth'); $call(); // expected fatal
    // $call = array('G','meth'); $call(); // expected fatal
    // $call = array('H','meth'); $call(); // expected fatal
    // $call = array('parent','meth'); $call(); // expected fatal
    // $call = array('self','meth'); $call();   // expected fatal
    // $call = array('static','meth'); $call(); // expected fatal
    echo "****************\n";

    // call_user_func(array('B','C::meth')); // expected fatal
    // call_user_func(array('B','D::meth')); // expected fatal
    // call_user_func(array('B','F::meth')); // expected fatal
    // call_user_func(array('B','G::meth')); // expected fatal
    // call_user_func(array('B','H::meth')); // expected fatal



    echo "****************\n";
    // $call = array('B','B::meth'); $call(); // expected fatal
    // $call = array('B','C::meth'); $call(); // expected fatal
    // $call = array('B','D::meth'); $call(); // expected fatal
    // $call = array('B','F::meth'); $call(); // expected fatal
    // $call = array('B','G::meth'); $call(); // expected fatal
    // $call = array('B','H::meth'); $call(); // expected fatal
    // $call = array('B','parent::meth'); $call(); // expected fatal
    // $call = array('B','self::meth'); $call();   // expected fatal
    // $call = array('B','static::meth'); $call(); // expected fatal
    echo "****************\n";
    // call_user_func(array('G','B::meth')); // expected fatal
    // call_user_func(array('G','C::meth')); // expected fatal
    // call_user_func(array('G','D::meth')); // expected fatal


    // call_user_func(array('G','H::meth')); // expected fatal



    echo "****************\n";
    // $call = array('G','B::meth'); $call(); // expected fatal
    // $call = array('G','C::meth'); $call(); // expected fatal
    // $call = array('G','D::meth'); $call(); // expected fatal
    // $call = array('G','F::meth'); $call(); // expected fatal
    // $call = array('G','G::meth'); $call(); // expected fatal
    // $call = array('G','H::meth'); $call(); // expected fatal
    // $call = array('G','parent::meth'); $call(); // expected fatal
    // $call = array('G','self::meth'); $call();   // expected fatal
    // $call = array('G','static::meth'); $call(); // expected fatal
    echo "****************\n";
    $b = new B;
    call_user_func(varray[$b,'meth']);    // B B B
    call_user_func(varray[$b,'B::meth']); // B B B
    call_user_func(varray[$b,'C::meth']); // warning
    call_user_func(varray[$b,'D::meth']); // warning
    call_user_func(varray[$b,'F::meth']); // warning
    call_user_func(varray[$b,'G::meth']); // warning
    call_user_func(varray[$b,'H::meth']); // warning
    call_user_func(varray[$b,'parent::meth']); // A B B
    call_user_func(varray[$b,'self::meth']);   // B B B
    call_user_func(varray[$b,'static::meth']); // warning
    echo "****************\n";
    $b = new B;
    $call = varray[$b,'meth']; $call();    // B B B
    $call = varray[$b,'B::meth']; $call(); // B B B
    // $call = array($b,'C::meth'); $call(); // expected fatal
    // $call = array($b,'D::meth'); $call(); // expected fatal
    // $call = array($b,'F::meth'); $call(); // expected fatal
    // $call = array($b,'G::meth'); $call(); // expected fatal
    // $call = array($b,'H::meth'); $call(); // expected fatal
    $call = varray[$b,'parent::meth']; $call(); // A B B
    $call = varray[$b,'self::meth']; $call();   // B B B
    // $call = array($b,'static::meth'); $call(); // expected fatal
    echo "****************\n";
    $g = new G;
    call_user_func(varray[$g,'meth']);    // G G G
    call_user_func(varray[$g,'B::meth']); // warning
    call_user_func(varray[$g,'C::meth']); // warning
    call_user_func(varray[$g,'D::meth']); // warning
    call_user_func(varray[$g,'F::meth']); // F G G
    call_user_func(varray[$g,'G::meth']); // G G G
    call_user_func(varray[$g,'H::meth']); // warning
    call_user_func(varray[$g,'parent::meth']); // F G G
    call_user_func(varray[$g,'self::meth']);   // G G G
    call_user_func(varray[$g,'static::meth']); // warning
    echo "****************\n";
    $g = new G;
    $call = varray[$g,'meth']; $call();    // G G G
    // $call = array($g,'B::meth'); $call(); // expected fatal
    // $call = array($g,'C::meth'); $call(); // expected fatal
    // $call = array($g,'D::meth'); $call(); // expected fatal
    $call = varray[$g,'F::meth']; $call(); // F G G
    $call = varray[$g,'G::meth']; $call(); // G G G
    // $call = array($g,'H::meth'); $call(); // expected fatal
    $call = varray[$g,'parent::meth']; $call(); // F G G
    $call = varray[$g,'self::meth']; $call();   // G G G
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
    call_user_func(parent::class.'::staticMeth'); // B D
    call_user_func(self::class.'::staticMeth');   // C D
    call_user_func(static::class.'::staticMeth'); // D D
    echo "****************\n";
    call_user_func(varray['B','staticMeth']); // B B   (Zend: B D) (Rule 3)
    call_user_func(varray['C','staticMeth']); // C C   (Zend: C D) (Rule 3)
    call_user_func(varray['D','staticMeth']); // D D
    call_user_func(varray['F','staticMeth']); // F F
    call_user_func(varray['G','staticMeth']); // G G
    call_user_func(varray['H','staticMeth']); // H H
    call_user_func(varray[parent::class,'staticMeth']); // B D
    call_user_func(varray[self::class,'staticMeth']);   // C D
    call_user_func(varray[static::class,'staticMeth']); // D D
    echo "****************\n";
    $call = varray['B','staticMeth']; $call(); // B B   (Zend: B D) (Rule 3)
    $call = varray['C','staticMeth']; $call(); // C C   (Zend: C D) (Rule 3)
    $call = varray['D','staticMeth']; $call(); // D D
    $call = varray['F','staticMeth']; $call(); // F F
    $call = varray['G','staticMeth']; $call(); // G G
    $call = varray['H','staticMeth']; $call(); // H H
    $call = varray['parent','staticMeth']; $call(); // B D
    $call = varray['self','staticMeth']; $call();   // C D
    $call = varray['static','staticMeth']; $call(); // D D
    echo "****************\n";
    call_user_func(varray['B','B::staticMeth']); // B B   (Zend: B D) (Rule 3)
    call_user_func(varray['B','C::staticMeth']); // warning
    call_user_func(varray['B','D::staticMeth']); // warning
    call_user_func(varray['B','F::staticMeth']); // warning
    call_user_func(varray['B','G::staticMeth']); // warning
    call_user_func(varray['B','H::staticMeth']); // warning
    call_user_func(varray['B','parent::staticMeth']); // A D
    call_user_func(varray['B','self::staticMeth']);   // B D
    call_user_func(varray['B','static::staticMeth']); // warning
    echo "****************\n";
    $call = varray['B','B::staticMeth']; $call(); // B B   (Zend: B D) (Rule 3)
    // $call = array('B','C::staticMeth'); $call(); // expected fatal
    // $call = array('B','D::staticMeth'); $call(); // expected fatal
    // $call = array('B','F::staticMeth'); $call(); // expected fatal
    // $call = array('B','G::staticMeth'); $call(); // expected fatal
    // $call = array('B','H::staticMeth'); $call(); // expected fatal
    $call = varray['B','parent::staticMeth']; $call(); // A D
    $call = varray['B','self::staticMeth']; $call();   // B D
    // $call = array('B','static::staticMeth'); $call(); // expected fatal
    echo "****************\n";
    call_user_func(varray['G','B::staticMeth']); // warning
    call_user_func(varray['G','C::staticMeth']); // warning
    call_user_func(varray['G','D::staticMeth']); // warning
    call_user_func(varray['G','F::staticMeth']); // F F
    call_user_func(varray['G','G::staticMeth']); // G G
    call_user_func(varray['G','H::staticMeth']); // warning
    call_user_func(varray['G','parent::staticMeth']); // F F   (Zend: F D) (Rule 4)
    call_user_func(varray['G','self::staticMeth']);   // G G   (Zend: G D) (Rule 4)
    call_user_func(varray['G','static::staticMeth']); // warning
    echo "****************\n";
    // $call = array('G','B::staticMeth'); $call(); // expected fatal
    // $call = array('G','C::staticMeth'); $call(); // expected fatal
    // $call = array('G','D::staticMeth'); $call(); // expected fatal
    $call = varray['G','F::staticMeth']; $call(); // F F
    $call = varray['G','G::staticMeth']; $call(); // G G
    // $call = array('G','H::staticMeth'); $call(); // expected fatal
    $call = varray['G','parent::staticMeth']; $call(); // F F   (Zend: F D) (Rule 4)
    $call = varray['G','self::staticMeth']; $call();   // G G   (Zend: G D) (Rule 4)
    // $call = array('G','static::staticMeth'); $call(); // expected fatal
    echo "****************\n";
    $b = new B;
    call_user_func(varray[$b,'staticMeth']);    // B B
    call_user_func(varray[$b,'B::staticMeth']); // B B
    call_user_func(varray[$b,'C::staticMeth']); // warning
    call_user_func(varray[$b,'D::staticMeth']); // warning
    call_user_func(varray[$b,'F::staticMeth']); // warning
    call_user_func(varray[$b,'G::staticMeth']); // warning
    call_user_func(varray[$b,'H::staticMeth']); // warning
    call_user_func(varray[$b,'parent::staticMeth']); // A B
    call_user_func(varray[$b,'self::staticMeth']);   // B B
    call_user_func(varray[$b,'static::staticMeth']); // warning
    echo "****************\n";
    $b = new B;
    $call = varray[$b,'staticMeth']; $call();    // B B
    $call = varray[$b,'B::staticMeth']; $call(); // B B
    // $call = array($b,'C::staticMeth'); $call(); // expected fatal
    // $call = array($b,'D::staticMeth'); $call(); // expected fatal
    // $call = array($b,'F::staticMeth'); $call(); // expected fatal
    // $call = array($b,'G::staticMeth'); $call(); // expected fatal
    // $call = array($b,'H::staticMeth'); $call(); // expected fatal
    $call = varray[$b,'parent::staticMeth']; $call(); // A B
    $call = varray[$b,'self::staticMeth']; $call();   // B B
    // $call = array($b,'static::staticMeth'); $call(); // expected fatal
    echo "****************\n";
    $g = new G;
    call_user_func(varray[$g,'staticMeth']);    // G G
    call_user_func(varray[$g,'B::staticMeth']); // warning
    call_user_func(varray[$g,'C::staticMeth']); // warning
    call_user_func(varray[$g,'D::staticMeth']); // warning
    call_user_func(varray[$g,'F::staticMeth']); // F G
    call_user_func(varray[$g,'G::staticMeth']); // G G
    call_user_func(varray[$g,'H::staticMeth']); // warning
    call_user_func(varray[$g,'parent::staticMeth']); // F G
    call_user_func(varray[$g,'self::staticMeth']);   // G G
    call_user_func(varray[$g,'static::staticMeth']); // warning
    echo "****************\n";
    $g = new G;
    $call = varray[$g,'staticMeth']; $call();    // G G
    // $call = array($g,'B::staticMeth'); $call(); // expected fatal
    // $call = array($g,'C::staticMeth'); $call(); // expected fatal
    // $call = array($g,'D::staticMeth'); $call(); // expected fatal
    $call = varray[$g,'F::staticMeth']; $call(); // F G
    $call = varray[$g,'G::staticMeth']; $call(); // G G
    // $call = array($g,'H::staticMeth'); $call(); // expected fatal
    $call = varray[$g,'parent::staticMeth']; $call(); // F G
    $call = varray[$g,'self::staticMeth']; $call();   // G G
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
    call_user_func(parent::class.'::staticMeth'); // B D
    call_user_func(self::class.'::staticMeth');   // C D
    call_user_func(static::class.'::staticMeth'); // D D
    echo "****************\n";
    call_user_func(varray['B','staticMeth']); // B B
    call_user_func(varray['C','staticMeth']); // C C
    call_user_func(varray['D','staticMeth']); // D D
    call_user_func(varray['F','staticMeth']); // F F
    call_user_func(varray['G','staticMeth']); // G G
    call_user_func(varray['H','staticMeth']); // H H
    call_user_func(varray[parent::class,'staticMeth']); // B D
    call_user_func(varray[self::class,'staticMeth']);   // C D
    call_user_func(varray[static::class,'staticMeth']); // D D
    echo "****************\n";
    $call = varray['B','staticMeth']; $call(); // B B
    $call = varray['C','staticMeth']; $call(); // C C
    $call = varray['D','staticMeth']; $call(); // D D
    $call = varray['F','staticMeth']; $call(); // F F
    $call = varray['G','staticMeth']; $call(); // G G
    $call = varray['H','staticMeth']; $call(); // H H
    $call = varray['parent','staticMeth']; $call(); // B D
    $call = varray['self','staticMeth']; $call();   // C D
    $call = varray['static','staticMeth']; $call(); // D D
    echo "****************\n";
    call_user_func(varray['B','B::staticMeth']); // B B
    call_user_func(varray['B','C::staticMeth']); // warning
    call_user_func(varray['B','D::staticMeth']); // warning
    call_user_func(varray['B','F::staticMeth']); // warning
    call_user_func(varray['B','G::staticMeth']); // warning
    call_user_func(varray['B','H::staticMeth']); // warning
    call_user_func(varray['B','parent::staticMeth']); // A D
    call_user_func(varray['B','self::staticMeth']);   // B D
    call_user_func(varray['B','static::staticMeth']); // warning
    echo "****************\n";
    $call = varray['B','B::staticMeth']; $call(); // B B
    // $call = array('B','C::staticMeth'); $call(); // expected fatal
    // $call = array('B','D::staticMeth'); $call(); // expected fatal
    // $call = array('B','F::staticMeth'); $call(); // expected fatal
    // $call = array('B','G::staticMeth'); $call(); // expected fatal
    // $call = array('B','H::staticMeth'); $call(); // expected fatal
    $call = varray['B','parent::staticMeth']; $call(); // A D
    $call = varray['B','self::staticMeth']; $call();   // B D
    // $call = array('B','static::staticMeth'); $call(); // expected fatal
    echo "****************\n";
    call_user_func(varray['G','B::staticMeth']); // warning
    call_user_func(varray['G','C::staticMeth']); // warning
    call_user_func(varray['G','D::staticMeth']); // warning
    call_user_func(varray['G','F::staticMeth']); // F F
    call_user_func(varray['G','G::staticMeth']); // G G
    call_user_func(varray['G','H::staticMeth']); // warning
    call_user_func(varray['G','parent::staticMeth']); // F F   (Zend: F D) (Rule 4)
    call_user_func(varray['G','self::staticMeth']);   // G G   (Zend: G D) (Rule 4)
    call_user_func(varray['G','static::staticMeth']); // warning
    echo "****************\n";
    // $call = array('G','B::staticMeth'); $call(); // expected fatal
    // $call = array('G','C::staticMeth'); $call(); // expected fatal
    // $call = array('G','D::staticMeth'); $call(); // expected fatal
    $call = varray['G','F::staticMeth']; $call(); // F F
    $call = varray['G','G::staticMeth']; $call(); // G G
    // $call = array('G','H::staticMeth'); $call(); // expected fatal
    $call = varray['G','parent::staticMeth']; $call(); // F F   (Zend: F D) (Rule 4)
    $call = varray['G','self::staticMeth']; $call();   // G G   (Zend: G D) (Rule 4)
    // $call = array('G','static::staticMeth'); $call(); // expected fatal
    echo "****************\n";
    $b = new B;
    call_user_func(varray[$b,'staticMeth']);    // B B
    call_user_func(varray[$b,'B::staticMeth']); // B B
    call_user_func(varray[$b,'C::staticMeth']); // warning
    call_user_func(varray[$b,'D::staticMeth']); // warning
    call_user_func(varray[$b,'F::staticMeth']); // warning
    call_user_func(varray[$b,'G::staticMeth']); // warning
    call_user_func(varray[$b,'H::staticMeth']); // warning
    call_user_func(varray[$b,parent::class.'::staticMeth']); // A B
    call_user_func(varray[$b,self::class.'::staticMeth']);   // B B
    call_user_func(varray[$b,static::class.'::staticMeth']); // warning
    echo "****************\n";
    $b = new B;
    $call = varray[$b,'staticMeth']; $call();    // B B
    $call = varray[$b,'B::staticMeth']; $call(); // B B
    // $call = array($b,'C::staticMeth'); $call(); // expected fatal
    // $call = array($b,'D::staticMeth'); $call(); // expected fatal
    // $call = array($b,'F::staticMeth'); $call(); // expected fatal
    // $call = array($b,'G::staticMeth'); $call(); // expected fatal
    // $call = array($b,'H::staticMeth'); $call(); // expected fatal
    $call = varray[$b,'parent::staticMeth']; $call(); // A B
    $call = varray[$b,'self::staticMeth']; $call();   // B B
    // $call = array($b,'static::staticMeth'); $call(); // expected fatal
    echo "****************\n";
    $g = new G;
    call_user_func(varray[$g,'staticMeth']);    // G G
    call_user_func(varray[$g,'B::staticMeth']); // warning
    call_user_func(varray[$g,'C::staticMeth']); // warning
    call_user_func(varray[$g,'D::staticMeth']); // warning
    call_user_func(varray[$g,'F::staticMeth']); // F G
    call_user_func(varray[$g,'G::staticMeth']); // G G
    call_user_func(varray[$g,'H::staticMeth']); // warning
    call_user_func(varray[$g,parent::class.'::staticMeth']); // F G
    call_user_func(varray[$g,self::class.'::staticMeth']);   // G G
    call_user_func(varray[$g,static::class.'::staticMeth']); // warning
    echo "****************\n";
    $g = new G;
    $call = varray[$g,'staticMeth']; $call();    // G G
    // $call = array($g,'B::staticMeth'); $call(); // warning
    // $call = array($g,'C::staticMeth'); $call(); // warning
    // $call = array($g,'D::staticMeth'); $call(); // warning
    $call = varray[$g,'F::staticMeth']; $call(); // F G
    $call = varray[$g,'G::staticMeth']; $call(); // G G
    // $call = array($g,'H::staticMeth'); $call(); // warning
    $call = varray[$g,'parent::staticMeth']; $call(); // F G
    $call = varray[$g,'self::staticMeth']; $call();   // G G
    // $call = array($g,'static::staticMeth'); $call(); // warning
    echo "****************\n";
  }
}

class D extends C {
  public function meth() {
    echo __CLASS__ . ' ' . static::class .
         (isset($this) ? ' '.get_class($this) : '') . "\n";
  }
  public static function staticMeth() {
    echo __CLASS__ . ' ' . static::class . "\n";
  }
}

class F {
  public function meth() {
    echo __CLASS__ . ' ' . static::class .
         (isset($this) ? ' '.get_class($this) : '') . "\n";
  }
  public static function staticMeth() {
    echo __CLASS__ . ' ' . static::class . "\n";
  }
}

class G extends F {
  public function meth() {
    echo __CLASS__ . ' ' . static::class .
         (isset($this) ? ' '.get_class($this) : '') . "\n";
  }
  public static function staticMeth() {
    echo __CLASS__ . ' ' . static::class . "\n";
  }
  public function doMeth() {
    $this->meth();  // G G G



    F::meth();      // F G G
    G::meth();      // G G G

    parent::meth(); // F G G
    self::meth();   // G G G
    static::meth(); // G G G
    echo "****************\n";
  }
  public function doStaticMeth() {

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
    echo __CLASS__ . ' ' . static::class .
         (isset($this) ? ' '.get_class($this) : '') . "\n";
  }
  public static function staticMeth() {
    echo __CLASS__ . ' ' . static::class . "\n";
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

<<__EntryPoint>>
function main_cuf() {
error_reporting(-1);
main();
}
