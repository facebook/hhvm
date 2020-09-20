<?hh

class Bla
{
}
<<__EntryPoint>> function main(): void {
$b = new Bla;

var_dump($b != null);
var_dump($b == null);
var_dump($b !== null);
var_dump($b === null);
}
