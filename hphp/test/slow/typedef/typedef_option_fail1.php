<?hh

type foo = ?int;
function bar(foo $k) :mixed{}
<<__EntryPoint>> function main(): void {
bar("fail");
}
