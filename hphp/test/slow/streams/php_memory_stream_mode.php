<?php

var_dump(stream_get_meta_data(fopen('php://memory', 'rnt'))["mode"]);
var_dump(stream_get_meta_data(fopen('php://memory', 'rw'))["mode"]);
var_dump(stream_get_meta_data(fopen('php://memory', 'rwaxc'))["mode"]);
var_dump(stream_get_meta_data(fopen('php://memory', 'r'))["mode"]);
var_dump(stream_get_meta_data(fopen('php://memory', 'r+'))["mode"]);
