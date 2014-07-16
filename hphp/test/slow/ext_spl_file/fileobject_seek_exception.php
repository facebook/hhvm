<?php

$file = new SplFileObject(__FILE__);

try {
  $file->seek(-2);
} catch (LogicException $e) {
  echo $e->getMessage();
}
