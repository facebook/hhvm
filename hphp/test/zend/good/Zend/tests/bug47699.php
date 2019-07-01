<?hh
class A {
    static function test($v='') {
        print_r(static::class);
    }
}
class B extends A {}
<<__EntryPoint>> function main(): void {
B::test();
spl_autoload_register('B::test');
new X();
}
