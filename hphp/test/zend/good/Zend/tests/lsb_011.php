<?hh

class Test1 {
    static function ok() {
        echo "bug";
    }
    static function test() {
        static::ok();
    }
}

class Test2 extends Test1 {
    static function ok() {
        echo "ok";
    }
}
<<__EntryPoint>> function main(): void {
Test2::test();
}
