<?hh
class A {

  private static $testPublicIA = 100;
  <<__Memoize>>
  public function testPublic() :mixed{
    $__lval_tmp_0 = self::$testPublicIA;
    self::$testPublicIA++;
    return $__lval_tmp_0;
  }

  private static $testProtectedIA = 110;
  <<__Memoize>>
  protected function testProtected() :mixed{
    $__lval_tmp_1 = self::$testProtectedIA;
    self::$testProtectedIA++;
    return $__lval_tmp_1;
  }

  private static $testPrivateIA = 120;
  <<__Memoize>>
  private function testPrivate() :mixed{
    $__lval_tmp_2 = self::$testPrivateIA;
    self::$testPrivateIA++;
    return $__lval_tmp_2;
  }

  <<__LSB>>
  private static $testNotMemoizedIA = 130;

  public function testNotMemoized() :mixed{
    $__lval_tmp_3 = static::$testNotMemoizedIA;
    static::$testNotMemoizedIA++;
    return $__lval_tmp_3;
  }

  <<__Memoize>>
  public function testPassesThis() :mixed{
    return A::fnThatTakesThis($this);
  }

  private static $testAsyncI = 140;

  <<__Memoize>>
  public async function testAsync() :Awaitable<mixed>{
    await RescheduleWaitHandle::create(1, 1); // simulate blocking I/O
    $__lval_tmp_4 = self::$testAsyncI;
    self::$testAsyncI++;
    return $__lval_tmp_4;
  }

  private static $testNotMemoizedOverrideI = 150;

  <<__Memoize>>
  public function testNotMemoizedOverride() :mixed{
    $__lval_tmp_5 = self::$testNotMemoizedOverrideI;
    self::$testNotMemoizedOverrideI++;
    return $__lval_tmp_5;
  }

  private static $testMemoizedOverrideI = 160;
  public function testMemoizedOverride() :mixed{
    $__lval_tmp_6 = self::$testMemoizedOverrideI;
    self::$testMemoizedOverrideI++;
    return $__lval_tmp_6;
  }

  public static function fnThatTakesThis(A $a) :mixed{
    return $a->testNotMemoized();
  }

  public function testA() :mixed{
    // Show that after the first run we're returning the cached result
    echo $this->testPublic().' ';
    echo $this->testPublic().' ';
    echo $this->testProtected().' ';
    echo $this->testProtected().' ';
    echo $this->testPrivate().' ';
    echo $this->testPrivate().' ';
    echo $this->testNotMemoized().' ';
    echo $this->testNotMemoized()."\n";
  }
}

class B extends A {

  private static $testPublicIB = 200;
  <<__Memoize>>
  public function testPublic() :mixed{
    $__lval_tmp_7 = self::$testPublicIB;
    self::$testPublicIB++;
    return $__lval_tmp_7;
  }

  private static $testProtectedIB = 210;
  <<__Memoize>>
  protected function testProtected() :mixed{
    $__lval_tmp_8 = self::$testProtectedIB;
    self::$testProtectedIB++;
    return $__lval_tmp_8;
  }

  private static $testPrivateIB = 220;
  <<__Memoize>>
  private function testPrivate() :mixed{
    $__lval_tmp_9 = self::$testPrivateIB;
    self::$testPrivateIB++;
    return $__lval_tmp_9;
  }

  private static $testNotMemoizedOverrideIB = 230;

  public function testNotMemoizedOverride() :mixed{
    $__lval_tmp_10 = self::$testNotMemoizedOverrideIB;
    self::$testNotMemoizedOverrideIB++;
    return $__lval_tmp_10;
  }

  private static $testMemoizedOverrideIB = 240;
  <<__Memoize>>
  public function testMemoizedOverride() :mixed{
    $__lval_tmp_11 = self::$testMemoizedOverrideIB;
    self::$testMemoizedOverrideIB++;
    return $__lval_tmp_11;
  }

  public function testB() :mixed{
    // Show that after the first run we're returning the cached result
    echo $this->testPublic().' ';
    echo $this->testPublic().' ';
    echo $this->testProtected().' ';
    echo $this->testProtected().' ';
    echo $this->testPrivate().' ';
    echo $this->testPrivate().' ';
    echo parent::testPublic().' ';
    echo parent::testPublic().' ';
    echo parent::testProtected().' ';
    echo parent::testProtected()."\n";
  }
}


<<__EntryPoint>>
function main_base() :mixed{
(new A())->testA();
// Test to make sure that a new object isn't reusing the results from the old
// object,
(new A())->testA();

// Test async
$a = new A();
echo HH\Asio\join($a->testAsync()).' ';
echo HH\Asio\join($a->testAsync())."\n";

// Test Inheritence
$b = new B();
$b->testA();
$b->testB();

// Test what happens when you override a non-memoized method with a memoized one
// and visa-versa
echo $b->testNotMemoizedOverride().' ';
echo $b->testNotMemoizedOverride().' ';
echo $b->testMemoizedOverride().' ';
echo $b->testMemoizedOverride()."\n";

// Test passing a function that passes $this as an argument. This caused the
// segfault in #5150421.
echo $a->testPassesThis().' ';
echo $a->testPassesThis();
}
