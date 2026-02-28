<?hh

class BasicClass {}

function dump_iterateable($obj)
:mixed{
    $reflection = new ReflectionClass($obj);
    var_dump($reflection->isIterateable());
}
<<__EntryPoint>> function main(): void {
$basicClass = new BasicClass();
$stdClass = new stdClass();

dump_iterateable($basicClass);
dump_iterateable($stdClass);
}
