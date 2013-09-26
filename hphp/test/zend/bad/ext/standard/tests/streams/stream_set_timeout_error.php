<?php
/* Prototype  : proto bool stream_set_timeout(resource stream, int seconds, int microseconds)
 * Description: Set timeout on stream read to seconds + microseonds 
 * Source code: ext/standard/streamsfuncs.c
 * Alias to functions: socket_set_timeout
 */

echo "*** Testing stream_set_timeout() : error conditions ***\n";

//Test stream_set_timeout with one more than the expected number of arguments
echo "\n-- Testing stream_set_timeout() function with more than expected no. of arguments --\n";

for ($i=0; $i<100; $i++) {
  $port = rand(10000, 65000);
  /* Setup socket server */
  $server = @stream_socket_server("tcp://127.0.0.1:$port");
  if ($server) {
    break;
  }
}
/* Connect to it */
$client = fsockopen("tcp://127.0.0.1:$port");

$seconds = 10;
$microseconds = 10;
$extra_arg = 10;
var_dump( stream_set_timeout($client, $seconds, $microseconds, $extra_arg) );

// Testing stream_set_timeout with one less than the expected number of arguments
echo "\n-- Testing stream_set_timeout() function with less than expected no. of arguments --\n";

$seconds = 10;
var_dump( stream_set_timeout($client) );


echo "\n-- Testing stream_set_timeout() function with a closed socket --\n";
fclose($client);
var_dump( stream_set_timeout($client, $seconds) );

echo "\n-- Testing stream_set_timeout() function with an invalid stream --\n";
var_dump( stream_set_timeout($seconds, $seconds) );

echo "\n-- Testing stream_set_timeout() function with a stream that does not support timeouts --\n";
$filestream = fopen(__FILE__, "r");
var_dump( stream_set_timeout($filestream, $seconds) );

fclose($filestream);
fclose($server);

echo "Done";
?>