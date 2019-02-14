<?hh

function f<reify T>(mixed $x) {
 var_dump($x is T);
}

f<int>("hello");
f<int>(1);
