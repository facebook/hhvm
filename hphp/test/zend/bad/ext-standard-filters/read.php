<?php
echo "-TEST\n";
class filter extends php_user_filter {
    function filter($in, $out, &$consumed, $closing)
    {
        $output = 0;
        while ($bucket = stream_bucket_make_writeable($in)) {
            $bucket->data = strtoupper($bucket->data);
            $consumed += $bucket->datalen;
            stream_bucket_append($out, $bucket);
            $output = 1;
        }
        if ($closing) {
            $bucket = stream_bucket_new($this->stream, "\n===close===\n");
            stream_bucket_append($out, $bucket);
            $output = 1;
        }
        return $output ? PSFS_PASS_ON : PSFS_FEED_ME;
    }
}
stream_filter_register("strtoupper", "filter")
   or die("Failed to register filter");

if ($f = fopen(__FILE__, "rb")) {
    stream_filter_append($f, "strtoupper");
    while (!feof($f)) {
        echo fread($f, 8192);
    }
    fclose($f);
}
echo "Done\n";
?>