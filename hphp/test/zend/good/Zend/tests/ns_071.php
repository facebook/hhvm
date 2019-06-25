<?hh

namespace foo;

class bar {
    public function __construct(array $x = NULL) {
        \var_dump($x);
    }
}
<<__EntryPoint>> function main(): void {
new bar(null);
new bar(new \stdclass);
}
