<?php

if (isset($_SERVER['WWW'])) {
  $_SERVER['PHP_ROOT'] = $_SERVER['WWW'];
} else if (isset($_SERVER['HPHP_FACEBOOK_WWW'])) {
  $_SERVER['PHP_ROOT'] = $_SERVER['HPHP_FACEBOOK_WWW'];
} else {
  die('$WWW or $HPHP_FACEBOOK_WWW needs to be set in your environment');
}

require_once $_SERVER['PHP_ROOT'].'/flib/__flib.php';
flib_init(FLIB_CONTEXT_SCRIPT);
require_module('employee/info');
require_module('employee/unixname');

$CodeErrorJS = $argv[1];
$SourceRoot = $argv[2];
$Command = isset($argv[3]) ? $argv[3] : null;
$CmdPrefix = is_dir("$SourceRoot/.git") ? "git --work-tree=$SourceRoot " : "";

$errors = json_decode(file_get_contents($CodeErrorJS), true);

if (!$Command) {
  $CurEmail = get_email(posix_getlogin());
  if (!$CurEmail) {
    die("Current user doesnt seem to have a facebook email address");
  }
}

$emails = array();
$reporting = array(
  'BadPHPIncludeFile',
  'PHPIncludeFileNotFound',
  'UseEvaluation',
  'UseUndeclaredVariable',
  'UseUndeclaredConstant',
  'UnknownClass',
  'UnknownBaseClass',
  'UnknownObjectMethod',
  'InvalidMagicMethod',
  'UnknownFunction',
  'BadConstructorCall',
  'DeclaredVariableTwice',
  'DeclaredConstantTwice',
  'BadDefine',
  'RequiredAfterOptionalParam',
  'RedundantParameter',
  'TooFewArgument',
  'TooManyArgument',
  'BadArgumentType',
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

$blames = array();
if ($Command != "--list") {
  $errs = array();
  foreach ($reporting as $type) {
    if (!isset($errors[1][$type])) continue;
    foreach ($errors[1][$type] as $error) {
      $file  = $error['c1'][0];
      $line0 = $error['c1'][1];
      $errs[$file][] = $line0;
    }
  }
  foreach ($errs as $file => $lines) {
    $blames[$file] = get_blame($file, $lines);
  }
}

$count = 0;
foreach ($reporting as $type) {
  if (!isset($errors[1][$type])) continue;
  if ($Command != "--list") { echo $type; }
  foreach ($errors[1][$type] as $error) {
    $file  = $error['c1'][0];
    $line0 = $error['c1'][1];
    $char0 = $error['c1'][2];
    $line1 = $error['c1'][3];
    $char1 = $error['c1'][4];
    $details = $error['d'];

    if ($Command == "--list") {
      echo "$file:$line0: $type:$details\n";
    } else {
      if (isset($blames[$file][$line0])) {
        $blame = $blames[$file][$line0];
        $body = isset($emails[$blame]) ? $emails[$blame] : '';
        $body .= "\n\n======================================================\n";
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
  }
  echo "\n";
}

if (!$Command) {
  foreach ($emails as $blame => $body) {
    echo "sending mail to $blame...\n";
    mail(get_email($blame), "[nemo] A bug's life ends here",
         "Hi, there,\n\n".
         "HipHop compiler might have found some bugs with PHP code you ".
         "were working on. Would you take a quick look to see if they are ".
         "real problems? Please do not ask me to review your change. ".
         "Ask someone who's familiar with the code :-) If it is not your bug, ".
         "please try to find the right person/group to fix the problem.\n\n".
         "Also, this tool is still under development. If you think this is ".
         "something we shouldn't be warning about, please let me know\n\n".
         "Thanks!\n".$body,
         "From: $CurEmail");
  }
} else if ($Command == "--dry-run") {
  foreach ($emails as $blame => $body) {
    var_dump($blame, $body);
  }
}

///////////////////////////////////////////////////////////////////////////////

function get_code_block($file, $line0, $char0, $line1, $char1) {
  global $SourceRoot;

  $ret = '';

  $start = $line0 - 4;
  if ($start < 1) $start = 1;
  $end = $line1 + 5;
  $lines = array();
  exec("sed -ne$start,${end}p $SourceRoot/$file", $lines);
  foreach ($lines as $line) {
    $ret .= sprintf("%5d  %s\n", $start++, $line);
  }

  return $ret;
}

function get_blame($file, $lines) {
  global $SourceRoot, $CmdPrefix;

  $cmd = $CmdPrefix .
    "svn blame $SourceRoot/$file 2>/dev/null | ".
    "sed -ne 's/^[0-9]\\+\\s\\+\\([a-zA-Z_0-9]\\+\\).*\$/\\1/'";
  foreach ($lines as $line) {
    $cmd .= " -e${line}p";
  }

  $result = array();
  exec($cmd, $result);
  return array_combine($lines, $result);
}

function get_email($unixname) {
  global $CurEmail;

  $uid = unixname_get_uid($unixname);
  if (!$uid) return $CurEmail;
  $email = employee_get_email($uid);
  if (!$email) return $CurEmail;
  return $email;
}
