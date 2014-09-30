<?hh

function ends_with($big, $little) {
  return substr($big, -strlen($little)) == $little;
}

$cmd = 'grep -R -l -P "[\x00-\x09]|[\x0B-\x1F]|[\x7F]" '.__DIR__.'/../';
$files = explode("\n", trim(shell_exec($cmd)));
foreach ($files as $file) {
  $file = realpath($file);
  $data = file_get_contents($file);

  if (!$data) {
    print "Can't read $file\n";
    continue;
  }

  // Carriage returns are the devil
  $data = str_replace("\r", '', $data);

  if (ends_with($file, '.gz') ||
      ends_with($file, '.zip') ||
      ends_with($file, '.bz2')
     ) {
    continue;
  } else if (ends_with($file, '.expectf')) {
    // This is expected
  } else if (ends_with($file, '.expect')) {
    print "Bad char found in expect: $file\n";
    // TODO move this to a expectf if we can
    continue;
  } else {
    print "Bad char found in unknown file: $file\n";
    continue;
  }

  // Escape the rest
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

  // If there is more than one regex back to back, just don't end the first

  $data = str_replace("%r%r", '', $data);
  print "Fixed up $file\n";
  file_put_contents($file, $data);
}
