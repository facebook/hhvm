<?hh
function ret_true($x) {
 return true;
}
<<__EntryPoint>>
function entrypoint_1758(): void {

  switch ($_POST) {
  case darray[]: echo 'empty array';
   break;
  case $_GET:   echo 'get';
   break;
  default: echo 'default';
  }
  switch ($_SERVER) {
  case darray[]: echo 'empty array';
   break;
  default: echo 'default';
  }
  switch ($_SERVER) {
  case ret_true($_SERVER['foo'] = 10): echo '1';
   break;
  case darray[];
   echo '2';
   break;
  default: echo '3';
  }
  var_dump($_SERVER['foo']);
}
