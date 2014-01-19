<?php

$s_msg_qnum = "msg_qnum";
$filename = tempnam('/tmp', 'vmmsgqueue');

$token = ftok($filename, "a");
if (msg_queue_exists($token)) { echo "queue exists\n"; exit(1); }

$queue = msg_get_queue($token);
var_dump(msg_queue_exists($token));

$pid = pcntl_fork();
if ($pid == 0) {
  $q = msg_get_queue($token);
  msg_send($q, 2, "start");
  msg_receive($q, 1, $type, 100, $msg);
  msg_send($q, 2, $msg); // echo
  exit(0);
}

msg_receive($queue, 2, $type, 100, $msg);
var_dump($msg);

msg_send($queue, 1, "ok");
msg_receive($queue, 2, $type, 100, $msg);
var_dump($msg);

$ret = msg_stat_queue($queue);
var_dump($ret[$s_msg_qnum]);
msg_set_queue($queue, array("msg_perm.mode" => 0666));

msg_remove_queue($queue);
pcntl_waitpid($pid, $status);

unlink($filename);
