<?hh

function f<reify T>(T $x) { echo "yes\n"; }

f<darray<int, int>>(array());
f<darray<int, int>>(darray[]);
f<darray<int, int>>(dict[]);
