<?hh

class C<reify Ta> {}
<<__EntryPoint>> function main(): void {
$c = new C<(int, string)>();

// just wildcard
var_dump($c is C<_>);

// tuple with wildcard
var_dump($c is C<(int, string)>);
var_dump($c is C<(int, _)>);
var_dump($c is C<(_, string)>);
var_dump($c is C<(_, _)>);
var_dump($c is C<(_, int)>);
}
