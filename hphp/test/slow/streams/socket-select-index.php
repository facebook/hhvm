<?hh


<<__EntryPoint>>
function main_socket_select_index() :mixed{
$msg = "Hello";
$len = strlen($msg);

$sockets = varray[];
socket_create_pair(AF_UNIX, SOCK_STREAM, 0, inout $sockets);
socket_write($sockets[0], $msg, $len);

$fdset = darray[ 1 => $sockets[1] ];
$write = $excep = varray[];
socket_select(inout $fdset, inout $write, inout $excep, 0, 100);
print_r($fdset);
}
