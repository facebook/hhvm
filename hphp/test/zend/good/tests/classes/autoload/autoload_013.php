<?hh

<<__EntryPoint>> function autoload_013(): void {
try {
    new ReflectionClass("UndefC");
}
catch (ReflectionException $e) {
    echo $e->getMessage();
}
}
