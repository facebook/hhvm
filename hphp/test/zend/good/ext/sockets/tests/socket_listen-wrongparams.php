<?hh <<__EntryPoint>> function main(): void {
$socket = socket_create(AF_UNIX, SOCK_STREAM, 0);
var_dump(socket_listen($socket));
}
