<?php

function main() {
  $name = tempnam(sys_get_temp_dir(), '1682');
  file_put_contents($name,
                    "put this in the txt file\n");
  $txt = file_get_contents($name);
  unlink($name);
  echo $txt;

  $name = tempnam(sys_get_temp_dir(), '1682');
  file_put_contents("compress.zlib://$name",
                    "put this in the zip file\n");
  $zip = file_get_contents("compress.zlib://$name");
  unlink($name);
  echo $zip;

  file_put_contents('php://stdout', "file_put_contents\n");
}
main();
