<?php
require 'server.inc';

function do_test($follow) {
  $options = [
    'http' => [
      'method' => 'POST',
      'follow_location' => $follow,
    ],
  ];

  $ctx = stream_context_create($options);

  $responses = [
    "data://text/plain,HTTP/1.1 308\r\nLocation: /foo\r\n\r\n",
    "data://text/plain,HTTP/1.1 200\r\nConnection: close\r\n\r\n",
  ];
  $pid = http_server('tcp://127.0.0.1:12342', $responses, $output);

  $fd = fopen('http://127.0.0.1:12342/', 'rb', false, $ctx);
  fseek($output, 0, SEEK_SET);
  echo stream_get_contents($output);

  http_server_kill($pid);
}

do_test(true);
do_test(false);

?>
Done
