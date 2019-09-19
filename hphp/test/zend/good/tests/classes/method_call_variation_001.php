<?hh
class C
{
    function foo($a, $b)
    {
        echo "Called C::foo($a, $b)\n";
    }
}

function foo($a, $b)
{
    echo "Called global foo($a, $b)\n";
}

<<__EntryPoint>> function main(): void {
$c = new C;

$functions = ['foo', [2 => [3 => array()]]];
$functions[1][2][3][4] = 'foo';

$c->$functions[0](1, 2);
$c->$functions[1][2][3][4](3, 4);

$c->functions = ['foo', [2 => [3 => array()]]];
$c->functions[1][2][3][4] = 'foo';

$c->functions[0](5, 6);
$c->functions[1][2][3][4](7, 8);
}
