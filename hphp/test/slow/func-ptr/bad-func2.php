<?hh

class A {
    public function f1($a) { return $a; }
}

<<__EntryPoint>>
function main() {
    $a = "A::f1";
    var_dump($a(1));

    $o = new A();
    $a = HH\inst_meth($o, "f1");
    var_dump($a(3));
    var_dump($a[1](4));
}
