<?hh // strict

interface A1 {
  <<__Rx, __Mutable>>
  public function f(): void;
  <<__Rx>>
  public function f1(<<__Mutable>>A1 $a): void;
}

interface B1 extends A1 {
  // OK to override mutable with mutable
  <<__Override, __Rx, __Mutable>>
  public function f(): void;

  // OK to override mutable with mutable
  <<__Override, __Rx>>
  public function f1(<<__Mutable>>A1 $a): void;
}

interface A2 {
  <<__Rx>>
  public function f(): void;
  <<__Rx>>
  public function f1(A2 $a): void;
}

interface B2 extends A2 {
  // OK to override immutable with immutable
  <<__Override, __Rx>>
  public function f(): void;
  <<__Override, __Rx>>
  public function f1(A2 $a): void;
}

interface A21 {
  <<__Rx, __MaybeMutable>>
  public function f(): void;
  <<__Rx>>
  public function f1(<<__MaybeMutable>>A2 $a): void;
}

interface B21 extends A21 {
  // OK to override maybe mutable with maybe mutable
  <<__Override, __Rx, __MaybeMutable>>
  public function f(): void;
  // OK to override maybe mutable with maybe mutable
  <<__Override, __Rx>>
  public function f1(<<__MaybeMutable>>A2 $a): void;
}

interface A3 {
  <<__Rx, __Mutable>>
  public function f(): void;
  <<__Rx>>
  public function f1(<<__Mutable>>A3 $a): void;
}

interface B3 extends A3 {
  // OK to override mutable with mutable-or-immutable
  <<__Override, __Rx, __MaybeMutable>>
  public function f(): void;
  // OK to override mutable with mutable-or-immutable
  <<__Override, __Rx>>
  public function f1(<<__MaybeMutable>>A3 $a): void;
}


interface A4 {
  <<__Rx>>
  public function f(): void;
  <<__Rx>>
  public function f1(A4 $a): void;
}

interface B4 extends A4 {
  // OK to override immutable with mutable-or-immutable
  <<__Override, __Rx, __MaybeMutable>>
  public function f(): void;
  // OK to override immutable with mutable-or-immutable
  <<__Override, __Rx>>
  public function f1(<<__MaybeMutable>>A4 $a): void;
}
