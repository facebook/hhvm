<?hh

type foo = ?int;
function bar(foo $k) {}
<<__EntryPoint>> function main(): void {
bar("fail");
}
