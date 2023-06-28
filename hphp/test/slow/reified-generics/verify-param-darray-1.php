<?hh

function f<reify T>(T $x) :mixed{ echo "yes\n"; }
<<__EntryPoint>> function main(): void {
f<darray<int, int>>(varray[]);
f<darray<int, int>>(darray[]);
f<darray<int, int>>(dict[]);
}
