<?hh
// Constants defined from moduleInit()
// such as AF_INET should still be
// visible to defined() even in repo mode
<<__EntryPoint>> function main(): void {
var_dump(defined("AF_INET"));
}
