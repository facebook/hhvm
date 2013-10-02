<?php

// originally based on ext/standard/tests/file/stream_rfc2397_006.phpt

$streams = array(
  "data:;base64,\0Zm9vYmFyIGZvb2Jhcg==",
  "data:;base64,Zm9vYmFy\0IGZvb2Jhcg==",
  "data:;base64,Zm9vYmFyIGZvb2Jhcg==",
  'data:;base64,#Zm9vYmFyIGZvb2Jhcg==',
  'data:;base64,#Zm9vYmFyIGZvb2Jhc=',
);

foreach ($streams as $stream) {
  var_dump(file_get_contents($stream));
}
