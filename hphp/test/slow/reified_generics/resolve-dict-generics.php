<?hh

class C {}

function f<reify T>() { echo "done\n"; }
<<__EntryPoint>> function main(): void {
f<dict<int, C>>();
}
