<?hh
class foo {
    public function __construct() {}
}

class bar extends foo {
}
<<__EntryPoint>> function main(): void {
echo (new ReflectionClass("bar"))->__toString();
}
