<?hh
class A {
    public function __call($m, $a) {
        echo "__call\n";
    }
}

class B extends A {
    public static function foo() {
        echo "foo\n";
    }
}
<<__EntryPoint>> function main(): void {
if (is_callable(varray['B', 'foo'])) {
    call_user_func(varray['B', 'foo']);
}
if (is_callable(varray['A', 'foo'])) {
    call_user_func(varray['A', 'foo']);
}
}
