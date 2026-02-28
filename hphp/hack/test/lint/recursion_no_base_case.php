<?hh
// @format

function foo1(): void {
  foo1(); //should produce lint
}
function foo2(): void {
  foo2(); //should produce lint
  $x = 1;
}
function foo3(): void {
  $x = 1; //should produce lint
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

function test_throw_lint(): Error {
  throw test_throw_lint(); // should produce lint
}

//Testing for conditionals

function test_if_return(): int {
  $x1 = 2;
  if ($x1 == 2) {
    return $x1;
  }
  return test_if_return(); //should not produce lint
}
function test_if_void(): void {
  $x1 = 2;
  if ($x1 == 2) {
    return;
  }
  test_if_void(); //should not produce lint
}

function test_multiple_base_cases_lint(int $x): int {
  if ($x == 1) {
    return test_multiple_base_cases_lint($x);
  } else if ($x == 2) {
    return test_multiple_base_cases_lint(1);
  }
  return test_multiple_base_cases_lint($x); //should produce lint
}

function test_multiple_base_cases_no_lint(int $x): int {
  if ($x == 1) {
    return 1;
  } else if ($x == 2) {
    return test_multiple_base_cases_no_lint(1);
  }
  return test_multiple_base_cases_no_lint($x); //should not produce lint
}

function test_multiple_base_cases_no_lint_throw(int $x): int {
  if ($x == 1) {
    throw new Error('error');
  } else if ($x == 2) {
    return test_multiple_base_cases_no_lint_throw(1);
  }
  return test_multiple_base_cases_no_lint_throw($x); //should not produce lint
}

function test_multiple_base_cases_lint_throw(int $x): Error {
  if ($x == 1) {
    throw test_multiple_base_cases_lint_throw($x);
  } else if ($x == 2) {
    return test_multiple_base_cases_lint_throw(1);
  }
  return test_multiple_base_cases_lint_throw($x); //should produce lint
}

function test_return_in_else_no_lint(int $x): int {
  if ($x == 1) {
    return test_return_in_else_no_lint(1);
    ;
  } else if ($x == 2) {
    return 2;
  }
  return test_return_in_else_no_lint($x); //should not produce lint
}

function test_nested_ifs_lint(int $x): int {
  if ($x == 1) {
    if ($x == 2) {
      return test_nested_ifs_lint($x);
    }
  } else if ($x == 2) {
    return test_nested_ifs_lint(1);
  }
  return test_nested_ifs_lint($x); //should produce lint
}

function test_nested_ifs_no_lint(int $x): int {
  if ($x == 1) {
    if ($x == 2) {
      return $x;
    }
  } else if ($x == 2) {
    return test_nested_ifs_no_lint(1);
  }
  return test_nested_ifs_no_lint($x); //should not produce lint
}

function test_no_return_no_lint(int $i): void {
  if ($i > 0) {
    echo "$i";
    test_no_return_no_lint($i - 1); //should not produce lint
  }
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
    Foo::bar5($x); //should produce lint
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
  public function returnBar5(): int {
    $x1 = 2;
    if ($x1 == 2) {
      return $x1;
    }
    return $this->returnBar5(); //should not produce lint
  }
  public function testMultipleBaseCasesLint(int $x): int {
    if ($x == 1) {
      return $this->testMultipleBaseCasesLint($x);
    } else if ($x == 2) {
      return $this->testMultipleBaseCasesLint(1);
    }
    return $this->testMultipleBaseCasesLint($x); //should produce lint
  }

  public function testMultipleBaseCasesNoLint(int $x): int {
    if ($x == 1) {
      return 1;
    } else if ($x == 2) {
      return $this->testMultipleBaseCasesNoLint(1);
    }
    return $this->testMultipleBaseCasesNoLint($x); //should not produce lint
  }

  public function testReturnInElseNoLint(int $x): int {
    if ($x == 1) {
      return $this->testReturnInElseNoLint(1);
      ;
    } else if ($x == 2) {
      return 2;
    }
    return $this->testReturnInElseNoLint($x); //should not produce lint
  }

  public function testNestedIfsLint(int $x): int {
    if ($x == 1) {
      if ($x == 2) {
        return $this->testNestedIfsLint($x);
      }
    } else if ($x == 2) {
      return $this->testNestedIfsLint(1);
    }
    return $this->testNestedIfsLint($x); //should produce lint
  }

  public function testNestedIfsNoLint(int $x): int {
    if ($x == 1) {
      if ($x == 2) {
        return $x;
      }
    } else if ($x == 2) {
      return $this->testNestedIfsNoLint(1);
    }
    return $this->testNestedIfsNoLint($x); //should not produce lint
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
