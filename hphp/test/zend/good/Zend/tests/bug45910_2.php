<?hh

class foo {
    const AAA = 'x';
    const BBB = 'a';
    const CCC = 'a';
    const DDD = self::AAA;

    private static $foo = darray[
        self::BBB    => 'a',
        self::CCC    => 'b',
        self::DDD    =>  11
    ];

    public static function test() {
        self::$foo;
    }
}
<<__EntryPoint>> function main(): void {
foo::test();

print 1;
}
