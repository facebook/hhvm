<?php
error_reporting(E_ALL | E_NOTICE);
$socket_file = "/tmp/" . str_repeat("a", 512);
function get_truncated_socket_filename($errno, $errmsg, $file, $line) {
    global $socket_file;
    print_r ($errmsg);
    preg_match("#maximum allowed length of (\d+) bytes#", $errmsg, $matches);
    $socket_file = substr($socket_file, 0, intval($matches[1]) - 1);
}
set_error_handler("get_truncated_socket_filename", E_NOTICE);
stream_socket_server("unix://" . $socket_file);
unlink($socket_file);
?>