<?hh // strict

class A implements HH\FunctionAttribute {
  public function __construct(public int $i) {}
}

class B implements HH\FunctionAttribute {
  public function __construct(public int $i) {}
}

<<A(3)>>
function f(): void {}

<<B(3)>>
function g(): void {}

<<B("wrong")>>
function h(): void {}
