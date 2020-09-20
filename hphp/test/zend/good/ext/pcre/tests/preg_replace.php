<?hh
<<__EntryPoint>> function main(): void {
var_dump(preg_replace('{{\D+}}', 'x', '{abcd}'));
var_dump(preg_replace('{{\D+}}', 'ddd', 'abcd'));

var_dump(preg_replace('/(ab)(c)(d)(e)(f)(g)(h)(i)(j)(k)/', 'a${1}2$103', 'zabcdefghijkl'));
}
