<?hh

function test($string) :mixed{
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

<<__EntryPoint>>
function main_stream_get_line_returnvalue() :mixed{
echo "<pre>";
test("one\n\nthree\nfour");
test("one\n\nthree\nfour\n");
test("\ntwo\nthree\nfour");
test("\ntwo\nthree\nfour\n");
test("one\ntwo\n\n\n");
}
