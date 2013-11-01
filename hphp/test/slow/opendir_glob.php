<?php

$dir = opendir('glob://' . __DIR__ . '/../sample_dir/*');

while ( ($file = readdir($dir)) !== false ) {
  echo "$file\n";
}
