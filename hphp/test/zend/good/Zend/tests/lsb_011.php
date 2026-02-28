<?hh

class Test1 {
    static function ok() :mixed{
        echo "bug";
    }
    static function test() :mixed{
        static::ok();
    }
}

class Test2 extends Test1 {
    static function ok() :mixed{
        echo "ok";
    }
}
<<__EntryPoint>> function main(): void {
Test2::test();
}
