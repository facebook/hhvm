<?php

spl_autoload_register(function ($class) {
    printf("loading\n");
    class TestFilter extends php_user_filter {
        public function filter($in, $out, &$consumed, $closing) {
            printf("filtering\n");
            while ($bucket = stream_bucket_make_writeable($in)) {
                $bucket->data = strtoupper($bucket->data);
                $consumed += $bucket->datalen;
                stream_bucket_append($out, $bucket);
            }
            return PSFS_PASS_ON;
        }
    }
}, true, false);

stream_filter_register('test_filter', '\TestFilter');

$stream = fopen('php://memory', 'r+');
stream_filter_append($stream, 'test_filter', STREAM_FILTER_WRITE);
fwrite($stream, "data");
