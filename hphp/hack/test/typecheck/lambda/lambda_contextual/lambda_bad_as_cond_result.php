<?hh // strict
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
  MyClosureRunner<MyClass> $x,
): MyClosureRunner<MyClass> {
  return $x;
}
function getThing1(bool $b): MyClosureRunner<MyClass> {
  return $b
    ? new MyClosureRunner(
        function($arg) {
          $arg->missing_method();
          return "ok";
        },
      )
    : new MyClosureRunner(
        function($arg) {
          $arg->another_missing_methodO();
          return "ok";
        },
      );
}

function breakIt(): void {
  $x = new MyClass();
  $c = getThing1(false)->get();
  $c($x);
}

function main(): void {
  breakIt();
}
