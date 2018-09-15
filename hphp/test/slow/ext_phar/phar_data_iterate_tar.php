<?php

function main() {
  $pd = new PharData(__DIR__.'/tgz/Archive_Tar-1.3.11.tgz');
  $out = array();
  $it = new RecursiveIteratorIterator($pd);
  foreach ($it as $fname => $data) {
    $out[$fname] = array(
      get_class($data),
      $data->getFilename(),
      $data->getPathname(),
      (string) $data
    );
  }
  ksort($out);
  var_dump($out);
}


<<__EntryPoint>>
function main_phar_data_iterate_tar() {
main();
}
