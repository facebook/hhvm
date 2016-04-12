<?php
// Trait binding after anon class
trait T {
    public function m1() { return 42; }
}
class C {
    public function m2() {
        return new class {};
    }
    use T;
}
$c = new C;
var_dump($c->m1());
?>
