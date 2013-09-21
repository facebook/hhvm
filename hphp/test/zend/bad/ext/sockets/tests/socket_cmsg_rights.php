<?php
include __DIR__."/mcast_helpers.php.inc";
$path = __DIR__ . "/unix_sock";

@unlink($path);

echo "creating send socket\n";
$sends1 = socket_create(AF_UNIX, SOCK_DGRAM, 0) or die("err");
var_dump($sends1);
socket_set_nonblock($sends1) or die("Could not put in non-blocking mode");

echo "creating receive socket\n";
$s = socket_create(AF_UNIX, SOCK_DGRAM, 0) or die("err");
var_dump($s);
$br = socket_bind($s, $path) or die("err");
var_dump($br);
socket_set_nonblock($s) or die("Could not put in non-blocking mode");

$r = socket_sendmsg($sends1, [
  "name" => [ "path" => $path ],
  "iov" => ["test ", "thing", "\n"],
  "control" => [
    [
      "level" => SOL_SOCKET,
      "type" => SCM_RIGHTS,
      "data" => [$sends1, STDIN, STDOUT, STDERR],
    ]
  ]
], 0);
var_dump($r);
checktimeout($s, 500);

$data = [
    "name" => [],
    "buffer_size" => 2000,
    "controllen" => socket_cmsg_space(SOL_SOCKET, SCM_RIGHTS, 3)
];
var_dump($data);
if (!socket_recvmsg($s, $data, 0)) die("recvmsg");
print_r($data);?>
<?php
$path = __DIR__ . "/unix_sock";
@unlink($path);
