<?php
class TestStream { //data, empty data, empty data + eof
    private $s = 0;
    function stream_open($path, $mode, $options, &$opened_path) {
            return true;
    }
    function stream_read($count) {
        echo "Read done\n";
        if ($this->s++ == 0)
            return "a\nbb\ncc";

        return "";
    }
    function stream_eof() {
        return $this->s >= 2;
    }

}

stream_wrapper_register("test", "TestStream");

$f = fopen("test://", "r");
while (!feof($f)) {
    $line = stream_get_line($f, 99, "\n");
    var_dump($line);
}
