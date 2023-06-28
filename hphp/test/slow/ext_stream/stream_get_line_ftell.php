<?hh

function test($string, $delim) :mixed{
    $stream = fopen('php://memory', 'r+');
    fwrite($stream, $string);
    rewind($stream);

    while (true) {
        $line = stream_get_line($stream, 666, $delim);
        if ($line === false) {
            break;
        }
        echo ftell($stream) . ": ";
        var_dump($line);
    }
    echo "\n";
}

<<__EntryPoint>>
function main_stream_get_line_returnvalue() :mixed{
    test("1\n12\n123", "\n");
    test("1\r\n12\r\n123", "\r\n");
}
