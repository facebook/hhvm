//// base-a.php
<?hh
class A {
  public function foo(): arraykey { return "s"; }
}
//// base-b.php
<?hh
class B extends A {
  public function foo(): int { return 5; }
}
//// base-c.php
<?hh
class C extends B {
  public function foo(): int { return 6; }

}
//// base-use-foo.php
<?hh
function accept_arraykey(arraykey $_): void {}
function use_a_foo(): void {
  accept_arraykey((new A())->foo());
}
function use_b_foo(): void {
  accept_arraykey((new B())->foo());
}
function use_c_foo(): void {
  accept_arraykey((new C())->foo());
}

//// changed-a.php
<?hh
class A {
  public function foo(): string { return "s"; }
}
//// changed-b.php
<?hh
class B extends A {
  public function foo(): int { return 5; }
}
//// changed-c.php
<?hh
class C extends B {
  public function foo(): int { return 6; }

}
//// changed-use-foo.php
<?hh
function accept_arraykey(arraykey $_): void {}
function use_a_foo(): void {
  accept_arraykey((new A())->foo());
}
function use_b_foo(): void {
  accept_arraykey((new B())->foo());
}
function use_c_foo(): void {
  accept_arraykey((new C())->foo());
}
