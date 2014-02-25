<?php
// #1909
// fwrite() failure behavior when used on a stream_socket_pair with one end
// closed.
function handler($errno, $errstr, $errfile, $errline) {
    var_dump($errno);
    var_dump($errstr);
    var_dump($errfile);
    var_dump($errline);
    return true;
}
list($a, $b) = stream_socket_pair(STREAM_PF_UNIX, STREAM_SOCK_STREAM,
                                  STREAM_IPPROTO_IP);
fclose($b);

set_error_handler("handler");
var_dump(fwrite($a, "foo"));

restore_error_handler();
fclose($a);
