<?hh

class C
{
    public $d;
}
<<__EntryPoint>> function main(): void {
$c = new C();

$arr = dict[1 => 'a', 2 => 'b', 3 => 'c'];

// Works fine:
foreach($arr as $x => $c->d)
{
    echo "{$x} => {$c->d}\n";
}

// Crashes:
foreach($arr as $c->d => $x)
{
    echo "{$c->d} => {$x}\n";
}
}
