//// module_M1.php
<?hh
new module M1 {}

//// module_M2.php
<?hh
new module M2 {}

//// A.php
<?hh

module M1;

class A {
  protected internal static function static_foo(): string {
    return "static_foo";
  }

  protected internal static int $static_bar = 42;

  protected internal function foo(): string {
    return "foo";
  }

  protected internal int $bar = 42;
}

//// B.php
<?hh

module M1;

class B extends A {
  public function foobar(): string {
    // OK since B extends A and resides in the same module
    A::$static_bar = 322;
    // OK since B extends A and resides in the same module
    A::static_foo().'bar';
    // OK since B extends A and resides in the same module
    $this->bar = 322;
    // OK since B extends A and resides in the same module
    return $this->foo().'bar';
  }
}

//// C.php
<?hh

module M1;

class C {
  public function foobar(): string {
    // Not OK since static_bar is protected
    A::$static_bar = 322;
    // Not OK since static_foo is protected
    A::static_foo().'bar';
    // Not OK since bar is protected
    (new A())->bar = 322;
    // Not OK since foo is protected
    return (new A())->foo().'bar';
  }
}

//// D.php
<?hh

module M2;

class D extends A {
  public function foobar(): string {
    // Not OK since static_bar is internal
    A::$static_bar = 322;
    // Not OK since static_foo is internal
    A::static_foo().'bar';
    // Not OK since bar is internal
    $this->bar = 322;
    // Not OK since foo is internal
    return $this->foo().'bar';
  }
}

//// E.php
<?hh

module M2;

class E {
  public function foobar(): string {
    // Not OK since static_bar is both protected and internal
    A::$static_bar = 322;
    // Not OK since static_foo is both protected and internal
    A::static_foo().'bar';
    // Not OK since bar is both protected and internal
    (new A())->bar = 322;
    // Not OK since foo is both protected and internal
    return (new A())->foo().'bar';
  }
}
