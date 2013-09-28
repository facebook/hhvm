<?php
$context = stream_context_create();

// Single option
var_dump(stream_context_set_option($context, 'http', 'method', 'POST'));

// Array of options
$options = array(
    'http' => array(
        'protocol_version' => 1.1,
        'user_agent'       => 'PHPT Agent',
    ),
);
var_dump(stream_context_set_option($context, $options));

var_dump(stream_context_get_options($context));
?>