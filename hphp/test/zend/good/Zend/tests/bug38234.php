<?hh
class Foo {
    function __clone() {
        throw new Exception();
    }
}
<<__EntryPoint>> function main(): void {
try {
    $x = new Foo();
    $y = clone $x;
} catch (Exception $e) {
}
echo "ok\n";
}
