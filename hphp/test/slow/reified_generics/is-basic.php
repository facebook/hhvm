<?hh

function f<reified T>(mixed $x) {
 var_dump($x is T);
}

f<reified int>("hello");
f<reified int>(1);
