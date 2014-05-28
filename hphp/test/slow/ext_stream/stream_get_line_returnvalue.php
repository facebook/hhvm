<?php
echo "<pre>";
test("one\n\nthree\nfour");
test("one\n\nthree\nfour\n");
test("\ntwo\nthree\nfour");
test("\ntwo\nthree\nfour\n");
test("one\ntwo\n\n\n");

function test($string) {
    $stream = fopen('php://memory', 'r+');
    fwrite($stream, $string);
    rewind($stream);

    for ($i = 0; $i < 6; $i ++) {
        $line = stream_get_line($stream, 666, "\n");
        echo "$i" . (feof($stream) ? ' (eof)' : '') . ": ";
        var_dump($line);
    }
    echo "\n";
}
