<?hh

class BasicClass {}

function dump_iterateable($obj)
{
    $reflection = new ReflectionClass($obj);
    var_dump($reflection->isIterateable());
}
<<__EntryPoint>> function main(): void {
$basicClass = new BasicClass();
$stdClass = new StdClass();

dump_iterateable($basicClass);
dump_iterateable($stdClass);
}
