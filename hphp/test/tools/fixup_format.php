<?hh

$ignore_extensions = array(
  'bmp',
  'bz2',
  'csv',
  'diff',
  'gd2',
  'gdf',
  'gif',
  'gz',
  'hhbbc',
  'hhbc',
  'ico',
  'iff',
  'jp2',
  'jpc',
  'jpeg',
  'jpg',
  'mo',
  'mp3',
  'odt',
  'otf',
  'pdf',
  'phar',
  'png',
  'ppt',
  'psd',
  'res',
  'swf',
  'tar',
  'tgz',
  'tif',
  'tiff',
  'ttf',
  'txt',
  'wbmp',
  'webp',
  'wsdl',
  'xbm',
  'zip',
);

$ignore_files = array(
  '/Zend/tests/multibyte/multibyte_encoding_003.php',
  '/Zend/tests/multibyte/multibyte_encoding_006.php',
  '/ext/fileinfo/tests/magic',
  '/ext/fileinfo/tests/magic',
  '/ext/fileinfo/tests/test_file_t4156480',
  '/ext/soap/tests/interop/Round4/GroupI/r4_groupI_xsd_006w.php',
  '/test/ext/test_zlib_file',
);

function ends_with($big, $little) {
  return substr($big, -strlen($little)) == $little;
}

// Everything but tab, newline and [' ' to 'Z']
$cmd = 'grep -R -l -P "[\x00-\x08]|[\x0B-\x1F]|[\x7F]" '.__DIR__.'/../';
$files = explode("\n", trim(shell_exec($cmd)));

foreach ($files as $file) {
  $file = realpath($file);
  foreach ($ignore_files as $ignore_file) {
    if (strpos($file, $ignore_file) !== false) {
      continue 2;
    }
  }
  foreach ($ignore_extensions as $ext) {
    if (ends_with($file, $ext)) {
      continue 2;
    }
  }

  $data = file_get_contents($file);
  if (!$data) {
    print "Can't read $file\n";
    continue;
  }

  // Carriage returns are the devil
  $data = str_replace("\r", '', $data);

  if (ends_with($file, '.expectf') || ends_with($file, '.expectf-repo')) {
    // Escape everything with %r
    for ($pos = 0; $pos < strlen($data); $pos++) {
      $char = $data[$pos];
      if (($char < "\n") ||
          ($char > "\n" && $char < ' ') ||
          ($char > '~')) {
        $data =
          substr($data, 0, $pos).'%r'.
          '\\x'.str_pad(dechex(ord($char)), 2, 0, STR_PAD_LEFT).
          '%r'.substr($data, $pos+1);
      }
    }
    $data = str_replace("%r%r", '', $data);

  } else if (ends_with($file, '.php')) {
    if (strpos($data, '__HALT_COMPILER') !== false) {
      // This file will be binary after this point. Don't touch it.
      continue;
    }
    // Only do the unix2dos thing done above already

  } else if (ends_with($file, '.skipif') ||
             ends_with($file, '.ini')) {
    // Only do the unix2dos thing done above already

  } else if (ends_with($file, '.expect')) {
    // TODO move this to a expectf if we can
    continue;
  } else {
    print "Bad char found in unknown file: $file\n";
    continue;
  }

  // If there is more than one regex back to back, just don't end the first

  print "Fixed up $file\n";
  file_put_contents($file, $data);
}
