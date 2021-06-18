<?hh

namespace foo;

interface foo {

}

class bar {
    public function __construct(foo $x = NULL) {
        \var_dump($x);
    }
}

class test implements foo {

}

<<__EntryPoint>> function main(): void {
new bar(new test);
new bar(null);
new bar(new \stdClass);
}
