<?hh

class B<reify Ta, reify Tb> {}

class C<reify Ta, reify Tb> extends B<Ta, int> {}
<<__EntryPoint>> function main(): void {
$x = new C<string, bool>();

echo "-- Valid input of reified\n";
var_dump($x is B<string, bool>);
var_dump($x is B<string, int>);

echo "-- Wildcards\n";
var_dump($x is B<int, _>);
var_dump($x is B<string, _>);
var_dump($x is B<_, bool>);
var_dump($x is B<_, int>);

echo "-- Wrong number\n";
var_dump($x is B<int, bool, int>);
var_dump($x is B<_, _, _>);
var_dump($x is B<_>);
var_dump($x is B<string>);
}
