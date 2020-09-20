<?hh

namespace foo;

class bar {
    public function __construct(\stdclass $x = NULL) {
        \var_dump($x);
    }
}
<<__EntryPoint>> function main(): void {
new bar(new \stdclass);
new bar(null);
}
