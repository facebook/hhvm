<?hh

function f<reify T>(mixed $x) :mixed{
 var_dump($x is T);
}
<<__EntryPoint>> function main(): void {
f<int>("hello");
f<int>(1);
}
