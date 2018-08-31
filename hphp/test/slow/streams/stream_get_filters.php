<?php


<<__EntryPoint>>
function main_stream_get_filters() {
var_dump(in_array('string.rot13', stream_get_filters()));
}
