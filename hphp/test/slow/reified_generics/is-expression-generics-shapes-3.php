<?hh

class C<reify Ta> {}
<<__EntryPoint>> function main(): void {
$c = new C<shape('a' => int, 'b' => string, ...)>();

// just wildcard
var_dump($c is C<_>);

// shape with wildcard
var_dump($c is C<shape('a' => int, 'b' => string)>);
var_dump($c is C<shape('b' => string, 'a' => int)>);
var_dump($c is C<shape('a' => _, 'b' => string)>);
var_dump($c is C<shape('a' => int, 'b' => _)>);
var_dump($c is C<shape('a' => string, 'b' => _)>);

// missing
var_dump($c is C<shape('a' => int)>);

// extra
var_dump($c is C<shape('a' => int, 'b' => string, 'c' => string)>);

// optional
var_dump($c is C<shape('a' => int, 'b' => string, ?'c' => string)>);
var_dump($c is C<shape(?'a' => int, 'b' => string)>);

// unknown fields
var_dump($c is C<shape('a' => int, 'b' => string, ...)>);
}
