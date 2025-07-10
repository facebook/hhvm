<?hh

module z.z;

<<__DynamicallyCallable>> function f(): string {
    return "in f";
}

class A {
    function __construct() {
    }

    static function b(): string {
        return "in b";
    }
}
