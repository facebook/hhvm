<?hh

<<file:__EnableUnstableFeatures('type_const_super_bound')>>

// Note: nothing <: int <: num <: mixed

abstract class A {
  abstract const type T super int;

  public function do(this::T $any): this::T { return $any; }

  public function takes_int(int $_): void {}
  public function returns_int(): int { return 42; }
}

abstract class B1 extends A {
  abstract const type T super num;
  abstract const type T2 super this::T;  // OK

  <<__Override>>
  public function takes_int(this::T $superInt): void {  // OK
    parent::takes_int($superInt);  // ERROR
    parent::do($superInt);  // OK
  }

  <<__Override>>
  public function do(this::T2 $superT): this::T {  // OK
    parent::do($superT);  // ERROR
    parent::takes_int($superT);  // ERROR
    return parent::returns_int();  // OK
  }

  <<__Override>>
  public function returns_int(): this::T {  // ERROR
    return 42;
  }
}

abstract class B2 extends A {
   <<__Override>>
   public function do(this::T $_): int { return 42; }  // OK
}
