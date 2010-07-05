<?php

define('PHP_ONLY',     1);
define('NON_PHP_ONLY', 2);
define('ALL_FILES',    3);
define('USE_DU',       4);

$arg = 0;
$dir = '.';            if ($argc > ++$arg) $dir            = $argv[$arg];
$file_type = PHP_ONLY; if ($argc > ++$arg) $file_type      = $argv[$arg];
$min_percentage = 2;   if ($argc > ++$arg) $min_percentage = $argv[$arg];
$ascii = false;        if ($argc > ++$arg) $ascii          = $argv[$arg];

if ($ascii) {
  define('CHAR_V', '|');
  define('CHAR_H', '.');
  define('CHAR_T', 'L');
  define('CHAR_L', 'L');
  define('LINE_INDENT', 4);
  define('WORD_INDENT', 2);
} else {
  define('CHAR_V', "\xe2\x94\x82");
  define('CHAR_H', "\xe2\x94\x80");
  define('CHAR_T', "\xe2\x94\x9c");
  define('CHAR_L', "\xe2\x94\x94");
  define('LINE_INDENT', 2);
  define('WORD_INDENT', 2);
}

// main
du_dir(realpath($dir));
print "\nTotal Size: ".(int)($grand_total/1024 + 0.5)."M\n";

function du_dir($dir, $indent = array()) {
  global $min_percentage, $grand_total;

  if (!is_dir($dir)) return;

  $total = 0;
  $sizes = get_file_sizes($dir, $total);
  if (empty($indent)) $grand_total = $total;

  arsort($sizes);
  $min_size = $grand_total * $min_percentage / 100;
  $selected = array();
  foreach ($sizes as $file => $size) {
    if ($size > $min_size) {
      $selected[$file] = $size;
    }
  }

  $index = 0;
  foreach ($selected as $file => $size) {
    $last = (++$index == count($selected));
    $percentage = (int)($size / $grand_total * 100 + 0.5);

    $mb = (int)($size / 1024 + 0.5);
    if (!empty($indent)) {
      $first = true;
      foreach ($indent as $vertical) {
        if ($first) {
          print str_repeat(' ', WORD_INDENT);
          $first = false;
        } else {
          print ($vertical ? CHAR_V : ' ') .
            str_repeat(' ', LINE_INDENT + WORD_INDENT);
        }
      }
      print ($last ? CHAR_L : CHAR_T) . str_repeat(CHAR_H, LINE_INDENT);
    }
    print "$file: ${mb}M ($percentage%)\n";

    $indent[] = !$last;
    du_dir("$dir/$file", $indent);
    array_pop($indent);
  }
}

function get_file_sizes($dir, &$total) {
  global $file_type;

  if ($file_type == USE_DU) {
    $lines = array();
    exec('du -L --exclude="*/.svn*" --exclude="*/.git*" --max-depth=1 '.$dir,
         $lines);

    $sizes = array();
    $total = 0;
    foreach ($lines as $line) {
      if (preg_match('/^([0-9]+)[ \t]+'.preg_quote($dir, '/').'\/(.*)$/',
                     $line, $m)) {
        $size = $m[1]; $file = $m[2];
        $sizes[$file] = $size;
        $total += $size;
      }
    }
    return $sizes;
  }

  $cmd = 'find -L '.$dir.' -type f'.
    ' -not -regex ".*/\.svn/.*" -not -regex ".*/\.git/.*"';
  if ($file_type == PHP_ONLY) {
    $cmd .= ' -regex ".*\.php" -o -regex ".*\.phpt"';
  } else if ($file_type == NON_PHP_ONLY) {
    $cmd .= ' -not -regex ".*\.php" -not -regex ".*\.phpt"';
  }
  $lines = array();
  exec($cmd, $lines);

  $sizes = array();
  $total = 0;
  foreach ($lines as $line) {
    preg_match('#'.preg_quote($dir, '#').'/([^/]+)#', $line, $m);
    $file = $m[1];
    $size = filesize($line) / 1024;
    if (isset($sizes[$file])) {
      $sizes[$file] += $size;
    } else {
      $sizes[$file] = $size;
    }
    $total += $size;
  }
  return $sizes;
}
