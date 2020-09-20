<?hh

type foo = ?int;
function bar(foo $k) { var_dump($k); }

class something {}
type blah = ?something;
function bar2(blah $k) { var_dump($k); }

<<__EntryPoint>>
function main_typedef_option() {
bar(12);
bar(null);
bar(42);
bar2(new something());
bar2(null);
bar2(new something());
}
