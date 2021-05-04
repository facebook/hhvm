//// base-a.php
<?hh
class X {}
class A {}
//// base-b.php
<?hh
class Y extends X {}
class B extends A {
    public function foo(): int { return 0; }
}

//// changed-a.php
<?hh
class X {}
class A {
    public function foo(): arraykey { return "s"; } // Added method
}
//// changed-b.php
<?hh
class Y extends X {}
class B extends A {
    public function foo(): int { return 0; }
}
