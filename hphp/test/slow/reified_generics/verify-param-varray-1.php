<?hh

function f<reify T>(T $x) { echo "yes\n"; }
<<__EntryPoint>> function main(): void {
f<varray<int>>(varray[]);
f<varray<int>>(varray[]);
f<varray<int>>(vec[]);
}
