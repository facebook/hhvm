<?hh

class C<reify Ta, reify Tb> {}
<<__EntryPoint>> function main(): void {
$x = new C<string, bool>();

echo "-- Valid input of reified\n";
var_dump($x is C<string, bool>);
var_dump($x is C<int, bool>);

echo "-- Wildcards\n";
var_dump($x is C<int, _>);
var_dump($x is C<bool, _>);
var_dump($x is C<string, _>);
var_dump($x is C<_, bool>);
var_dump($x is C<_, int>);

echo "-- Wrong number\n";
var_dump($x is C<int, bool, int>);
var_dump($x is C<_, _, _>);
var_dump($x is C<_>);
var_dump($x is C<string>);
}
