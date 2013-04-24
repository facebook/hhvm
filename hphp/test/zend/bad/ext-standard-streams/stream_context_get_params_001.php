<?php

$ctx = stream_context_create();
var_dump($ctx);
var_dump(stream_context_get_params($ctx));

var_dump(stream_context_set_option($ctx, "foo","bar","baz"));
var_dump(stream_context_get_params($ctx));

var_dump(stream_context_set_params($ctx, array("notification" => "stream_notification_callback")));
var_dump(stream_context_get_params($ctx));

var_dump(stream_context_set_params($ctx, array("notification" => array("stream","notification_callback"))));
var_dump(stream_context_get_params($ctx));

var_dump(stream_context_get_params($ctx));
var_dump(stream_context_get_options($ctx));
var_dump(stream_context_get_params($ctx));
var_dump(stream_context_get_options($ctx));

?>