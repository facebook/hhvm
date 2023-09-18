<?hh

module z.z;

function f(): string {
    return "in f";
}

class A {
    function __construct() {
    }

    static function b(): string {
        return "in b";
    }
}
