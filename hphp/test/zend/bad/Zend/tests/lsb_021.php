<?php
class A {
    public static function test() {
        echo get_called_class()."\n";
    }
}

class B extends A {
    public static function testForward() {
        parent::test();
        call_user_func("parent::test");
        call_user_func(array("parent", "test"));
        self::test();
        call_user_func("self::test");
        call_user_func(array("self", "test"));
    }
    public static function testNoForward() {
        A::test();
        call_user_func("A::test");
        call_user_func(array("A", "test"));
        B::test();
        call_user_func("B::test");
        call_user_func(array("B", "test"));
    }
}

class C extends B {

}

C::testForward();
C::testNoForward();

?>