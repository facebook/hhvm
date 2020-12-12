<?hh

function f<reify T>(mixed $x) {
 var_dump($x is T);
}
<<__EntryPoint>> function main(): void {
f<int>("hello");
f<int>(1);
}
