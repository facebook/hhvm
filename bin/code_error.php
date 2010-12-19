<?php

$CodeErrorJS = $argv[1];
$SourceRoot = $argv[2];
$EmailDomain = $argv[3];

$errors = json_decode(file_get_contents($CodeErrorJS), true);

$emails = array();
$reporting = array(
  'BadPHPIncludeFile',
  'PHPIncludeFileNotFound',
  //'UseEvaluation',
  //'UseUndeclaredVariable',
  'UseUndeclaredConstant',
  //'UnknownClass',
  'UnknownBaseClass',
  //'UnknownObjectMethod',
  'InvalidMagicMethod',
  'UnknownFunction',
  'BadConstructorCall',
  'DeclaredVariableTwice',
  //'DeclaredConstantTwice',
  'BadDefine',
  'RequiredAfterOptionalParam',
  'RedundantParameter',
  'TooFewArgument',
  'TooManyArgument',
  //'BadArgumentType',
  'StatementHasNoEffect',
  'UseVoidReturn',
  'MissingObjectContext',
  'MoreThanOneDefault',
  'InvalidArrayElement',
  'InvalidDerivation',
  'ReassignThis',
  'MissingAbstractMethodImpl',
  'BadPassByReference',
);
$count = 0;
foreach ($reporting as $type) {
  if (!isset($errors[1][$type])) continue;
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
      echo ".";
    } else {
      echo "x";
    }
  }
  echo "\n";
}

foreach ($emails as $blame => $body) {
  echo "sending mail to $blame...\n";
  $blame .= '@'.$EmailDomain;
  mail($blame, "[nemo] A bug's life ends here",
       "Hi, there\n\n".
       "HipHop compiler might have found some bugs with PHP code you were working on. Would you take a quick look to see if they are real problems?\n".$body);
}

///////////////////////////////////////////////////////////////////////////////

function get_code_block($file, $line0, $char0, $line1, $char1) {
  global $SourceRoot;

  $ret = '';

  // at most 5 lines ahead
  $end = $line1;
  $count = $line1 - $line0 + 5;
  $lines = array();
  exec("head -$end $SourceRoot/$file | tail -$count", $lines);
  $n = $end - count($lines) + 1;
  foreach ($lines as $line) {
    $ret .= sprintf("%5d  %s\n", $n++, $line);
  }

  // at most 5 lines after
  $end = $line1 + 5;
  $count = 5;
  $lines = array();
  exec("head -$end $SourceRoot/$file | tail -$count", $lines);
  foreach ($lines as $line) {
    $ret .= sprintf("%5d  %s\n", $n++, $line);
  }

  return $ret;
}

function get_blame($file, $line) {
  global $SourceRoot;

  $cmd = "svn blame $SourceRoot/$file 2>/dev/null | head -$line | tail -1";
  $ret = shell_exec($cmd);
  if (preg_match('/^[0-9]+ +(\w+)/', $ret, $m)) {
    return $m[1];
  }
}
