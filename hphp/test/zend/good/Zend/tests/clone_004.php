<?hh

class foo {
    public function __get($a) {
        return new $this;
    }
}
<<__EntryPoint>> function main(): void {
error_reporting(E_ALL|E_STRICT);

$c = new foo;

$a = clone $c->b[1];
}
