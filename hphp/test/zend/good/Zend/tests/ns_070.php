<?hh

namespace foo;

class bar {
    public function __construct(\stdClass $x = NULL) {
        \var_dump($x);
    }
}
<<__EntryPoint>> function main(): void {
new bar(new \stdClass);
new bar(null);
}
