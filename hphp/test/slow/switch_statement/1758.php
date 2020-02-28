<?hh

switch ($_POST) {
case varray[]: echo 'empty array';
 break;
case $_GET:   echo 'get';
 break;
default: echo 'default';
}
switch ($_SERVER) {
case varray[]: echo 'empty array';
 break;
default: echo 'default';
}
function ret_true($x) {
 return true;
}
switch ($_SERVER) {
case ret_true($_SERVER['foo'] = 10): echo '1';
 break;
case varray[];
 echo '2';
 break;
default: echo '3';
}
var_dump($_SERVER['foo']);
