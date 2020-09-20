<?hh

class Foo { }
<<__EntryPoint>> function main(): void {
error_reporting(E_ALL|E_STRICT);

try {
    throw new Foo();
} catch (Foo $e) {
    var_dump($e);
}
}
