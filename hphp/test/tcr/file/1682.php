<?php


function main() {
  file_put_contents("/tmp/temp.txt",
                    "put this in the txt file\n");
  $txt = file_get_contents("/tmp/temp.txt");
  echo $txt;
  file_put_contents("compress.zlib:///tmp/temp.zip",
                    "put this in the zip file\n");
  $zip = file_get_contents("compress.zlib:///tmp/temp.zip");
  echo $zip;
  file_put_contents('php://stdout', "file_put_contents\n");
}
main();
