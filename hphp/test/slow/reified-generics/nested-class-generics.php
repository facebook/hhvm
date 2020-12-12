<?hh

class C<reify T> {}
class D<reify Ta, reify Tb> {}
<<__EntryPoint>> function main(): void {
$c = new C<D<int,string>>();

echo "true\n";
var_dump($c is C<_>);
var_dump($c is C<D<_, _>>);
var_dump($c is C<D<int, string>>);
var_dump($c is C<D<_, string>>);
var_dump($c is C<D<int, _>>);

echo "false\n";
var_dump($c is C<D<_>>);
var_dump($c is C<D<bool, string>>);
var_dump($c is C<D<string, int>>);
var_dump($c is C<D>);
}
