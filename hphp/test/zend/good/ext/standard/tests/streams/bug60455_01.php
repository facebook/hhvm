<?hh

//It's critical the read on the stream returns the input but doesn't set EOF
//flag the first time. This is why we need to use sockets.
<<__EntryPoint>> function main(): void {
$domain = (strtoupper(substr(PHP_OS, 0, 3)) == 'WIN' ? STREAM_PF_INET : STREAM_PF_UNIX);
($sockets = stream_socket_pair($domain, STREAM_SOCK_STREAM, STREAM_IPPROTO_IP))
        || exit("stream_socket_pair");
fwrite($sockets[0], "a");
stream_socket_shutdown($sockets[0], STREAM_SHUT_RDWR);

$f = $sockets[1];
while (!feof($f)) {
    $line = stream_get_line($f, 99, "\n");
    var_dump($line);
}
}
