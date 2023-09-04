//// base-a.php
<?hh
class A {}
//// base-b.php
<?hh
class B extends A {}

//// changed-a.php
<?hh
class A {
    public function foo(): void {}
}
//// changed-b.php
<?hh
class B extends A {}
