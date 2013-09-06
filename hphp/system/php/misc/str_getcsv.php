<?php

function str_getcsv(
  $input,
  $delimiter = ',',
  $enclosure = '"',
  $escape = '\\',
) {
  $args = array($delimiter, $enclosure, $escape);
  $defaults = array(',', '"', '\\');

  $args_len = count($args);
  for ($i = 0; $i < $args_len; ++$i) {
    $arg_len = strlen($args[$i]);

    // fgetcsv returns false if passed anything but a single char for its
    // last three args. str_getcsv's behavior is to truncate strings down
    // to their first char, and to turn invalid args into the default args.
    if ($arg_len === 0) {
      $args[$i] = $defaults[$i];
    } else if ($arg_len > 1) {
      $args[$i] = substr($args[$i], 0, 1);
    }
  }

  $temp = tmpfile();
  fwrite($temp, $input);
  fseek($temp, 0);
  $ret = fgetcsv($temp, 0, $args[0], $args[1], $args[2]);
  fclose($temp);
  return $ret !== false ? $ret : array(null);
}
