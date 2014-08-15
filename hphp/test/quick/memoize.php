<?php
class A {
  <<__Memoize>>
  public function testPublic() { static $i = 100; return $i++; }
  <<__Memoize>>
  protected function testProtected() { static $i = 110; return $i++; }
  <<__Memoize>>
  private function testPrivate() { static $i = 120; return $i++; }

  public function testNotMemoized() { static $i = 130; return $i++; }

  <<__Memoize>>
  public async function testAsync() {
    static $i = 140;
    await RescheduleWaitHandle::Create(1, 1); // simulate blocking I/O
    return $i++;
  }

  public function testA() {
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
  <<__Memoize>>
  public function testPublic() { static $i = 200; return $i++; }
  <<__Memoize>>
  protected function testProtected() { static $i = 210; return $i++; }
  <<__Memoize>>
  private function testPrivate() { static $i = 220; return $i++; }

  public function testB() {
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

(new A())->testA();
// Test to make sure that a new object isn't reusing the results from the old
// object,
(new A())->testA();

// Test async
$a = new A();
echo $a->testAsync()->join().' ';
echo $a->testAsync()->join()."\n";

// Test Inheritence
$b = new B();
$b->testA();
$b->testB();
