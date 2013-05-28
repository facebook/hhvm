<?php

$info = new SplFileInfo('does-not-exist-will-fail-on-getLinkTarget');
//readlink('does-not-throw-but-warns');
try{
  $info->getLinkTarget();
}
catch (Exception $e) {
  echo 'Caught exception: ',  $e->getMessage(), "\n";
  return;
}
echo "failed to throw\n";
