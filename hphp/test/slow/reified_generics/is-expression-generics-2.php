<?hh

class C<reify Ta, Tb, reify Tc> {}
<<__EntryPoint>> function main(): void {
$c = new C<int, bool, bool>();

echo "-- Only wildcards\n";
var_dump($c is C<_, _, _>);
var_dump($c is C<_>);
var_dump($c is C<_, _>);

echo "-- Some wildcards\n";
var_dump($c is C<_, string, _>);
var_dump($c is C<int, _, string>);
var_dump($c is C<int, _, bool>);
}
