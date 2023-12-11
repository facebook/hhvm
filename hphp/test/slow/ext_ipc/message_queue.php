<?hh


<<__EntryPoint>>
function main_message_queue() :mixed{
$s_msg_qnum = "msg_qnum";
$filename = tempnam(sys_get_temp_dir(), 'vmmsgqueue');

$token = ftok($filename, "a");
if (msg_queue_exists($token)) { echo "queue exists\n"; exit(1); }

$queue = msg_get_queue($token);
var_dump(msg_queue_exists($token));
$type = null;
$msg = null;
$errcode = null;

$pid = pcntl_fork();
if ($pid == 0) {
  $q = msg_get_queue($token);
  msg_send($q, 2, "start", true, true, inout $errcode);
  msg_receive($q, 1, inout $type, 100, inout $msg, true, 0, inout $errcode);
  msg_send($q, 2, $msg, true, true, inout $errcode); // echo
  exit(0);
}

msg_receive($queue, 2, inout $type, 100, inout $msg, true, 0, inout $errcode);
var_dump($msg);

msg_send($queue, 1, "ok", true, true, inout $errcode);
msg_receive($queue, 2, inout $type, 100, inout $msg, true, 0, inout $errcode);
var_dump($msg);

try {
  $ret = @msg_send($queue, 0, 'msg', false, false, inout $s_error_code);
  var_dump($ret);
} catch (UndefinedVariableException $e) {
  var_dump($e->getMessage());
}
try {
  var_dump(22 === $s_error_code); // 22 - invalid argument
} catch (UndefinedVariableException $e) {
  var_dump($e->getMessage());
}

$r_error_code = null;
$ret = msg_receive(
  $queue,
  0,
  inout $type,
  100,
  inout $msg,
  false,
  MSG_IPC_NOWAIT,
  inout $r_error_code
);
var_dump($ret);
var_dump(MSG_ENOMSG === $r_error_code);

$ret = msg_stat_queue($queue);
var_dump($ret[$s_msg_qnum]);
msg_set_queue($queue, dict["msg_perm.mode" => 0666]);
$qb = () ==> msg_stat_queue($queue)['msg_qbytes'];
msg_set_queue($queue, dict['msg_qbytes' => $qb() - 1]);
var_dump($qb());

msg_remove_queue($queue);
$status = null;
pcntl_waitpid($pid, inout $status);

unlink($filename);
}
