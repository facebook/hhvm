<?hh

function f<reify T>(mixed $x) {
 var_dump($x is T);
}

f<reify int>("hello");
f<reify int>(1);
