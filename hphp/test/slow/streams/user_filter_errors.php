<?php
var_dump(stream_get_filters("a"));
var_dump(stream_filter_register());
var_dump(stream_filter_append());
var_dump(stream_filter_prepend());
var_dump(stream_filter_remove());

var_dump(stream_bucket_make_writeable());
var_dump(stream_bucket_append());
var_dump(stream_bucket_prepend());
