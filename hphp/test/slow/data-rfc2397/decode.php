<?php

// originally based on ext/standard/tests/file/stream_rfc2397_003.phpt

$streams = array(
  'data://,A%20brief%20note',
  'data://application/vnd-xxx-query,select_vcount,fcol_from_fieldtable/local',
  'data://;base64,Zm9vYmFyIGZvb2Jhcg==',
);

foreach($streams as $original => $stream) {
  var_dump(file_get_contents($stream));
}
