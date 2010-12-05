<?php

$CodeErrorJS = $argv[1];
$SourceRoot = $argv[2];

$errors = json_decode(file_get_contents($CodeErrorJS), true);

$emails = array();
$reporting = array('TooFewArgument', 'TooManyArgument');
$count = 0;
foreach ($reporting as $type) {
  echo $type;
  foreach ($errors[1][$type] as $error) {
    $file  = $error['c1'][0];
    $line0 = $error['c1'][1];
    $char0 = $error['c1'][2];
    $line1 = $error['c1'][3];
    $char1 = $error['c1'][4];
    $details = $error['d'];

    $blame = get_blame($file, $line0);
    if ($blame) {
      $body = isset($emails[$blame]) ? $emails[$blame] : '';
      $body .= "\n\n=======================================================\n";
      $body .= "Last updated by: $blame\n";
      $body .= "File: $file\n";
      $body .= "Line: $line0\n";
      $body .= "Type: $type\nError: $details\nCode:\n\n";
      $body .= get_code_block($file, $line0, $char0, $line1, $char1);
      $emails[$blame] = $body;
    }

    echo ".";
    if (++$count > 100) {
      break;
    }
  }
  echo "\n";
}

foreach ($emails as $blame => $body) {
  mail($blame, "[nemo] A bug's life ends here",
       "Hi, there\n\n".
       "HipHop compiler might have found some bugs with PHP code you were working on. Would you take a quick look to see if they are real problems?\n".$body);
}

///////////////////////////////////////////////////////////////////////////////

function get_code_block($file, $line0, $char0, $line1, $char1) {
  global $SourceRoot;

  $end = $line1 + 5;
  $count = $line1 - $line0 + 10;
  return shell_exec("head -$end $SourceRoot/$file | tail -$count");
}

function get_blame($file, $line) {
  global $SourceRoot;

  $cmd = "svn blame $SourceRoot/$file 2>/dev/null | head -$line | tail -1";
  if (preg_match('/^[0-9]+ (\w+)/', shell_exec($cmd), $m)) {
    return $m[1];
  }
}
