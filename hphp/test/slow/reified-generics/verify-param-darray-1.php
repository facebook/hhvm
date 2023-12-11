<?hh

function f<reify T>(T $x) :mixed{ echo "yes\n"; }
<<__EntryPoint>> function main(): void {
f<darray<int, int>>(vec[]);
f<darray<int, int>>(dict[]);
f<darray<int, int>>(dict[]);
}
