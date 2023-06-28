<?hh

function send($msg_id, $msg) :mixed{
  $errcode = NULL;
  msg_send($msg_id, 1, $msg, true, true, inout $errcode);
}

function receive($msg_id) :mixed{
  $size = 0x7fffffffffffffff; // MAX_INT_64
  $msg_type = 10;
  $msg = NULL;
  $error_code = NULL;
  var_dump(msg_receive($msg_id, 1, inout $msg_type, $size, inout $msg, true, 0, inout $error_code));
  var_dump($error_code);
  var_dump(strlen($msg));
}

<<__EntryPoint>>
function main() :mixed{
  $msg_id = msg_get_queue(619158, 0600);

  send($msg_id, "hello");
  receive($msg_id);

  send($msg_id, "");
  receive($msg_id);

  send($msg_id, str_repeat("*", 1024));
  receive($msg_id);
}
