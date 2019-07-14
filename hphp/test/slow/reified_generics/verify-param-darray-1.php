<?hh

function f<reify T>(T $x) { echo "yes\n"; }
<<__EntryPoint>> function main(): void {
f<darray<int, int>>(array());
f<darray<int, int>>(darray[]);
f<darray<int, int>>(dict[]);
}
