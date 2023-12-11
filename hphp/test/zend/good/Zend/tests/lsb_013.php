<?hh

class Test1 {
    static function test() :mixed{
        var_dump(is_callable(static::class."::ok"));
        var_dump(is_callable(vec[static::class,"ok"]));
    }
}

class Test2 extends Test1 {
    static function ok() :mixed{
    }
}
<<__EntryPoint>> function main(): void {
Test1::test();
Test2::test();
}
