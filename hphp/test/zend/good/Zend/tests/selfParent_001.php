<?hh
class A {
    const myConst = "const in A";
    const myDynConst = self::myConst;

    public static function test() :mixed{
        var_dump(self::myDynConst);
    }
}

class B extends A {
    const myConst = "const in B";

    public static function test() :mixed{
        var_dump(parent::myDynConst);
    }
}
<<__EntryPoint>> function main(): void {
A::test();
B::test();
}
