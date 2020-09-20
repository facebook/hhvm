<?hh
<<__EntryPoint>> function main(): void {
var_dump(realpath('.') == realpath(getcwd()));
chdir('..');
var_dump(realpath('.') == realpath(getcwd()));
}
