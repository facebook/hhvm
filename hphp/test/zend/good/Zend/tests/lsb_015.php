<?hh
function foo() :mixed{
    B::throwException();
}
class C {
    public static function bla() :mixed{
        B::throwException();
    }
    public static function getException() :mixed{
        return new Exception();

    }
}
class A {

    public static function throwException_after() :mixed{
        C::bla();
    }
    public static function throwException() :mixed{
        throw C::getException();
    }
    public static function test() :mixed{
        static::who();
    }
    public static function who() :mixed{
        echo "A\n";
    }

    public static function mycatch() :mixed{
        try {
            static::who();
            B::throwException_after();
        } catch(Exception $e) {
            static::who();
            A::test();
            static::who();
            B::test();
            static::who();

            self::simpleCatch();
            static::who();
        }
    }

    public static function simpleCatch() :mixed{
        try {
            static::who();
            throw new Exception();
        } catch (Exception $e) {
            static::who();
        }
    }
}

class B extends A {
    public static function who() :mixed{
        echo "B\n";
    }

}
<<__EntryPoint>> function main(): void {
echo "via A:\n";
A::mycatch();
echo "via B:\n";
B::mycatch();
echo "==DONE==";
}
