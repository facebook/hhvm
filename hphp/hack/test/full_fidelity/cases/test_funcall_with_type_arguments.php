<?hh // strict

// ficticious functions with size-discernable names
f('hi'); // normal function call w/o generics
fo<string>('hello'); // annotated function call
foo(0 < 0, tuple()); // not generics annotated, but seems so a little
fooo(Bar < 0, 10 > Qux); // certainly not annotated, but hard to tell
foooo(bar<int,string>()); // annotated inside an argument list
