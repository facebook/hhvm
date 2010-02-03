#!/usr/local/bin/php
<?php

define('MSG_MAX_SIZE', 10 * 1024 * 1024);

$p = $argv[1];
$q = msg_get_queue(ftok($p, 'a'));
if ($q === false) {
  die('failed to get the message queue at '.$p);
}

if (!msg_send($q, 2, "CRUTCH")) {
  die('failed to send startup signal');
}

$Objects = array(); // All resources or objects we have created so far
$ObjectIndex = 1;
while (true) {
  $msg = null;
  if (msg_receive($q, 1, $type, MSG_MAX_SIZE, $msg)) {
    if ($type <= 0) break;

    $i = 0;
    $func   = $msg[$i++];
    $schema = $msg[$i++];
    $count  = $msg[$i++];
    $args   = $msg[$i++];

    if ($schema) {
      foreach ($schema as $index => $param) {
        if ($index >= 0 && $param[0] == 'O' && $args[$index]) {
          $original_msg[$index] = $args[$index];
          $args[$index] = $Objects[$args[$index]];
        }
      }
    }

    $expr = '$ret = $func(';
    for ($i = 0; $i < $count; $i++) {
      if ($i > 0) $expr .= ',';
      $expr .= '$args['.$i.']';
    }
    $expr .= ');';
    eval($expr);

    if ($schema) {
      $refs = array();
      foreach ($schema as $index => $param) {
        if ($param == 'R' /* Reference */) {
          $refs[$index] = $args[$index];
        } else if ($param == 'OO' /* Object/Resource Output */) {
          if ($index < 0) {
            if ($ret) {
              $Objects[$ObjectIndex] = $ret;
              $ret = $ObjectIndex++;
            }
          } else {
            $newobj = $args[$index];
            $oldobj = $Objects[$original_msg[$index]];
            if ($newobj && $newobj !== $oldobj) {
              $Objects[$ObjectIndex] = $newobj;
              $refs[$index] = $ObjectIndex++;
            }
          }
        }
      }
    }

    $response = array($ret, $refs);
    if (!msg_send($q, 2, $response)) break;
  }
}
