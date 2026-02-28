<?hh

function test(PDO $a = null, $b = 0, AnyArray $c) :mixed{}
<<__EntryPoint>> function main(): void {
$r = new ReflectionFunction('test');
foreach ($r->getParameters() as $p) {
    var_dump($p->isDefaultValueAvailable());
}

foreach ($r->getParameters() as $p) {
    if ($p->isDefaultValueAvailable()) {
        var_dump($p->getDefaultValue());
    }
}
}
