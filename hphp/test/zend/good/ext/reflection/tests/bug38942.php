<?hh
class foo {
    public function __construct() {}
}

class bar extends foo {
}
<<__EntryPoint>> function main(): void {
ReflectionClass::export("bar");
}
