<?php

foreach(array('UTF-8', 'uCs-2') as $targetCharset) {
  $s = fopen('php://memory', 'rw');
  fwrite($s, "\xe9"); // euro currency symbol in ISO-8859-1
  fseek($s, 0);
  stream_filter_append($s, "convert.iconv.ISO-8859-1/$targetCharset");
  $utf8Symbol = stream_get_contents($s);
  echo 'Euro symbol in ' . $targetCharset;
  echo ' has content "' . bin2hex($utf8Symbol) . '"';
  echo ' and size ' . strlen($utf8Symbol) . 'b', "\n";
}
