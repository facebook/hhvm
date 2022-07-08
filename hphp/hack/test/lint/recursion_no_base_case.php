<?hh

function foo1(): void {
  foo1(); // stack overflow, produces lint
}
function foo2(): void {
  foo2(); // stack overflow, produces lint
  $x = 1;
}
function foo3(): void {
  $x = 1; //should not produce lint
  foo3();
}
function foo4(int $x): void {
  foo4($x); //should produce lint
}
function foo5(): void {
  foo3(); //should not produce lint
}
async function foo6(): Awaitable<void> {
  await foo6(); //should produce lint
}

class Foo {
  public static function bar(): void {
    Foo::bar(); //should produce lint
  }
  public static function bar2(): void {
    static::bar2(); //should produce lint
  }
  public function bar4(): void {
     $this->bar4(); //should produce lint
  }
  public static function bar5(int $x): void {
    Foo::bar5(123); //should produce lint
  }
  public static function returnBar(): int {
    return Foo::returnBar(); //should produce lint
  }
  public static function returnBar2(): int {
    return static::returnBar2(); //should produce lint
  }

  public static function returnBar3(): int {
    return self::returnBar3(); //should produce lint
  }

  public function returnBar4(): int {
     return $this->returnBar4(); //should produce lint
  }

  public async function returnAwaitExpr(): Awaitable<void> {
     return await $this->returnAwaitExpr(); //should produce lint
  }
}

class Test1 {
  public static function testfun(): int {
    return 1;
  }
}

class Test2 {
  public function testfun(): int {
    return Test1::testfun(); //should not produce lint
  }
}
