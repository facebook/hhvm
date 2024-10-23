//// base-a.php
<?hh
class A {
  public function __construct(int $_) {}
}
//// base-b.php
<?hh
class B extends A {}
//// base-c.php
<?hh
class C extends B {}
//// base-f.php
<?hh
function f() : void {
  new C(0);
}

//// changed-a.php
<?hh
class A {
  public function __construct(int $_) {}
}
//// changed-b.php
<?hh
class B extends A {
  public function __construct(int $x, string $_) {
    parent::__construct($x);
  }
}
//// changed-c.php
<?hh
class C extends B {}
//// changed-f.php
<?hh
function f() : void {
  new C(0);
}
