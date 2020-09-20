<?hh
class A {
    public static function test() {
        echo static::class."\n";
    }
}

class B extends A {
    public static function testForward() {
        parent::test();
        call_user_func(parent::class."::test");
        call_user_func(varray[parent::class, "test"]);
        self::test();
        call_user_func(self::class."::test");
        call_user_func(varray[self::class, "test"]);
    }
    public static function testNoForward() {
        A::test();
        call_user_func("A::test");
        call_user_func(varray["A", "test"]);
        B::test();
        call_user_func("B::test");
        call_user_func(varray["B", "test"]);
    }
}

class C extends B {

}
<<__EntryPoint>> function main(): void {
C::testForward();
C::testNoForward();
}
