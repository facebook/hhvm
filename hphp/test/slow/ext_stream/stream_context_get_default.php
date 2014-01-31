<?php
var_dump(stream_context_get_options(stream_context_get_default()));
var_dump(stream_context_get_options(stream_context_get_default(
  array("http" => array("header" => "X-Hello: world")))));
var_dump(stream_context_get_options(stream_context_get_default()));
var_dump(stream_context_get_options(stream_context_set_default(
  array("http" => array("method" => "POST")))));
var_dump(stream_context_get_options(stream_context_get_default()));
