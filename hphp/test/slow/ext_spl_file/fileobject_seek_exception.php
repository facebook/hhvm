<?php


<<__EntryPoint>>
function main_fileobject_seek_exception() {
$file = new SplFileObject(__FILE__);

try {
  $file->seek(-2);
} catch (LogicException $e) {
  echo $e->getMessage();
}
}
