<?hh

<<__EntryPoint>> function main(): void {
try {
    new ReflectionClass("UndefC");
}
catch (ReflectionException $e) {
    echo $e->getMessage();
}
}
