<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class MyClass {
  public function hello(): string {
    return 'world';
  }
}

class MyClosureRunner<-T> {
  public function __construct(private (function(T): string) $closure) {}
  public function get(): (function(T): string) {
    return $this->closure;
  }
}

class MyClassClosureRunner {
  public function __construct(private (function(MyClass): string) $closure) {}
}

function expectsMyClosureRunnerMyClass(
  shape(
    'a' => MyClosureRunner<MyClass>,
    'b' => int,
  ) $p,
): MyClosureRunner<MyClass> {
  return $p['a'];
}
function getThing1(): MyClosureRunner<MyClass> {
  $var = expectsMyClosureRunnerMyClass(
    shape(
      'a' => new MyClosureRunner(
        function($arg) {
          $arg->missing_method();
          return "ok";
        },
      ),
      'b' => 23,
    ),
  );
  return $var;
}

function breakIt(): void {
  $x = new MyClass();
  $c = getThing1()->get();
  $c($x);
}

function main(): void {
  breakIt();
}
