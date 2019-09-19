<?hh

class A {
    static public function f1($a) { return $a; }

    static public function test() {
      var_dump(HH\class_meth(__CLASS__, 'f1'));
      var_dump(HH\class_meth(self::class, 'f1'));
    }
}

<<__EntryPoint>>
function main() {
    $a = 'A';
    $f1 = 'f1';
    var_dump(HH\class_meth(A::class, 'f1'));
    var_dump(HH\class_meth($a, 'f1'));
    var_dump(HH\class_meth('A', $f1));
    var_dump(HH\class_meth(A::class, $f1));
    var_dump(HH\class_meth($a, $f1));

    A::test();
}
