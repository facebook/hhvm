<?hh

function f<reify T>(T $x) { echo "yes\n"; }

f<varray<int>>(array());
f<varray<int>>(varray[]);
f<varray<int>>(vec[]);
