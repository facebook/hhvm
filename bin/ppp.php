<?php
define('STARTER_MARKER',  "namespace hphp_impl_starter {}\n");
define('SPLITTER_MARKER', "namespace hphp_impl_splitter {}\n");

$inputDir = preg_replace('#/$#', '', $argv[1]); // stripping trailing slash

$files = array();
exec("cd $inputDir && find cpp -name *.cpp", $files);

$sizes = array();
$clusterSize = calculate_cluster_size($sizes, $inputDir, $files);

$merges = $splits = $noops = array();
compute_merge_splits($merges, $splits, $sizes, $clusterSize);

// hzhao: I didn't find merge helped that much, so output splits only
print "splits {\n";
foreach ($splits as $file => $count) {
  print "  * {\n";
  print "    name = $file\n";
  print "    count = $count\n";
  print "  }\n";
}
print "}\n";

///////////////////////////////////////////////////////////////////////////////

function calculate_cluster_size(&$sizes, $inputDir, $files) {
  $total = 0;
  $sizes = array();
  foreach ($files as $file) {
    $pp = shell_exec("make -C $inputDir $file.pp");
    $pos = strpos($pp, STARTER_MARKER);
    if ($pos === false) {
      exit("Unable to find ImplStarter mark in $file\n");
    }
    $size = strlen($pp) - $pos;
    $sizes[$file] = $size;
    $total += $size;
  }
  exec("make -C $inputDir clobber");

  return (int)($total / count($sizes));
}

function compute_merge_splits(&$merges, &$splits, $sizes, $clusterSize) {
  $merge = array(); $merge_size = 0;
  foreach ($sizes as $file => $size) {
    if ($size >= $clusterSize) {
      $splits[$file] = ceil($size / $clusterSize);
    } else if ($size < $clusterSize) {
      if ($merge_size + $size > $clusterSize) {
        $merges[] = $merge;
        $merge = array();
        $merge_size = 0;
      }
      $merge[$file] = $size;
      $merge_size += $size;
    } else {
      $noops[$file] = $size;
    }
  }
  if ($merge) {
    $merges[] = $merge;
  }
}

function merge_files($inputDir, $merges) {
  $i = 0;
  foreach ($merges as $merge) {
    if (count($merge) > 1) {
      ++$i;
      $target = sprintf("cpp/merge.%03d.cpp", $i);
      $f = fopen("$inputDir/$target", "w+");

      $new_contents = '';
      foreach ($merge as $file => $size) {
        $content = file_get_contents("$inputDir/$file");
        $pos = strpos($content, STARTER_MARKER);
        fwrite($f, $content, $pos);
        $new_contents .= substr($content, $pos + strlen(STARTER_MARKER));
      }

      fwrite($f, $new_contents);
      fclose($f);
      system("cd $inputDir && rm -f " . implode(' ', array_keys($merge)));
    }
  }
}

function split_files($inputDir, $splits) {
  foreach ($splits as $file => $count) {
    $content = file_get_contents("$inputDir/$file");
    $header_size = $pos = strpos($content, STARTER_MARKER);
    $pos0 = $pos + strlen(STARTER_MARKER);

    $chunk_size = (int)((strlen($content) - $pos) / $count);
    for ($i = 0; $i < $count; $i++) {
      $pos = $pos0 + $chunk_size;
      $pos = @strpos($content, SPLITTER_MARKER, $pos);
      if ($pos == false) {
        $pos = strlen($content);
      }
      $f = fopen("$inputDir/$file.$i.cpp", "w+");
      fwrite($f, $content, $header_size);

      if ($i > 0) fwrite($f, "namespace HPHP {\n");
      fwrite($f, substr($content, $pos0, $pos - $pos0));

      $pos0 = $pos + strlen(SPLITTER_MARKER);
      if ($pos0 > strlen($content)) {
        fclose($f);
        break;
      }

      fwrite($f, "}\n");
      fclose($f);
    }

    system("rm -f $inputDir/$file");
  }
}
