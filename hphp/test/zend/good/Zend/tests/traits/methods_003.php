<?hh

trait foo {
    public function __construct() {
        var_dump(__FUNCTION__);
    }
}

class bar {
    use foo;
}
<<__EntryPoint>> function main(): void {
new bar;
}
