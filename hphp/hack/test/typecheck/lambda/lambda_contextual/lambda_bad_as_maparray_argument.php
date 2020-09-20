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
  array<string, MyClosureRunner<MyClass>> $p,
): MyClosureRunner<MyClass> {
  $r = $p['a'];
  return $r;
}
function getThing1(): MyClosureRunner<MyClass> {
  $var = expectsMyClosureRunnerMyClass(
    darray[
      'a' => new MyClosureRunner(
        function($arg) {
          $arg->missing_method();
          return "ok";
        },
      ),
    ],
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
