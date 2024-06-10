<?hh

class Test {
    public static function gen() :AsyncGenerator<mixed,mixed,void>{
        var_dump(get_class());
        var_dump(static::class);
        yield 1;
        yield 2;
        yield 3;
    }
}

class ExtendedTest extends Test {
}
<<__EntryPoint>> function main(): void {
foreach (ExtendedTest::gen() as $i) {
    var_dump($i);
}
}
