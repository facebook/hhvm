<?hh

<<__EntryPoint>>
function main_issue2601error() :mixed{
$flags = STREAM_CLIENT_CONNECT;
$uri = 'unix:///socket/not/found';

$errno = null;
$errstr = null;
$resource = @stream_socket_client($uri, inout $errno, inout $errstr, 60.0, $flags);

if (!$resource) {
    echo "Error: $errstr ($errno)";
}
}
