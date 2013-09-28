<?php
function do_stuff($url) {
    $handle=curl_init('http://127.0.0.1:37349/');
    curl_setopt($handle, CURLOPT_VERBOSE, true);
    curl_setopt($handle, CURLOPT_RETURNTRANSFER, true);
    curl_setopt($handle, CURLOPT_STDERR, $o = fopen($url, "w+"));
    curl_exec($handle);
    echo "About to rewind!\n";
    rewind($o);
    echo stream_get_contents($o);
    return $o;
}

echo "temp stream (close after):\n";
fclose(do_stuff("php://temp"));

echo "\nmemory stream (close after):\n";
fclose(do_stuff("php://memory"));

echo "\ntemp stream (leak):\n";
leak_variable(do_stuff("php://temp"), true);

echo "\nmemory stream (leak):\n";
leak_variable(do_stuff("php://memory"), true);

echo "\nDone.\n";