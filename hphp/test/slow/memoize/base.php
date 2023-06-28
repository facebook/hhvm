<?hh
class A {

  private static $testPublicIA = 100;
  <<__Memoize>>
  public function testPublic() :mixed{ return self::$testPublicIA++; }

  private static $testProtectedIA = 110;
  <<__Memoize>>
  protected function testProtected() :mixed{ return self::$testProtectedIA++; }

  private static $testPrivateIA = 120;
  <<__Memoize>>
  private function testPrivate() :mixed{ return self::$testPrivateIA++; }

  <<__LSB>>
  private static $testNotMemoizedIA = 130;

  public function testNotMemoized() :mixed{ return static::$testNotMemoizedIA++; }

  <<__Memoize>>
  public function testPassesThis() :mixed{ return A::fnThatTakesThis($this); }

  private static $testAsyncI = 140;

  <<__Memoize>>
  public async function testAsync() :Awaitable<mixed>{
    await RescheduleWaitHandle::create(1, 1); // simulate blocking I/O
    return self::$testAsyncI++;
  }

  private static $testNotMemoizedOverrideI = 150;

  <<__Memoize>>
  public function testNotMemoizedOverride() :mixed{ return self::$testNotMemoizedOverrideI++; }

  private static $testMemoizedOverrideI = 160;
  public function testMemoizedOverride() :mixed{ return self::$testMemoizedOverrideI++; }

  public static function fnThatTakesThis(A $a) :mixed{ return $a->testNotMemoized(); }

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
  public function testPublic() :mixed{ return self::$testPublicIB++; }

  private static $testProtectedIB = 210;
  <<__Memoize>>
  protected function testProtected() :mixed{ return self::$testProtectedIB++; }

  private static $testPrivateIB = 220;
  <<__Memoize>>
  private function testPrivate() :mixed{ return self::$testPrivateIB++; }

  private static $testNotMemoizedOverrideIB = 230;

  public function testNotMemoizedOverride() :mixed{ return self::$testNotMemoizedOverrideIB++; }

  private static $testMemoizedOverrideIB = 240;
  <<__Memoize>>
  public function testMemoizedOverride() :mixed{ return self::$testMemoizedOverrideIB++; }

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
