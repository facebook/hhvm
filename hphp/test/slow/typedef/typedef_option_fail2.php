<?hh

class something {}
type blah = ?something;
function bar2(blah $k) {}
<<__EntryPoint>> function main(): void {
bar2("fail");
}
