//// base-a.php
<?hh
class A {}
class X {}
//// base-b.php
<?hh
class B extends A {
    public function foo(): int { return 0; }
}
class Y extends X {}

//// changed-a.php
<?hh
class A {
    public function foo(): arraykey { return "s"; } // Added method
}
class X {}
//// changed-b.php
<?hh
class B extends A {
    public function foo(): int { return 0; }
}
class Y extends X {}
