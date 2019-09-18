<?hh
class Foo {
    public $test = "ok";
}
<<__EntryPoint>> function main(): void {
$class = new ReflectionClass("Foo");
$props = $class->getDefaultProperties();
echo $props["test"];
}
