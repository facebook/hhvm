<?hh

<<__EntryPoint>> function main(): void {
try {
    new ReflectionProperty('UndefC', 'p');
}
catch (ReflectionException $e) {
    echo $e->getMessage();
}
}
