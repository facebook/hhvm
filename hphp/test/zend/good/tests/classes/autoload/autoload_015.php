<?hh

<<__EntryPoint>> function autoload_015(): void {
try {
    new ReflectionProperty('UndefC', 'p');
}
catch (ReflectionException $e) {
    echo $e->getMessage();
}
}
