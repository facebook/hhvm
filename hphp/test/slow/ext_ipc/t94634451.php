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
  // Verify empty, small, and large queue messages use their exact lengths.
  $filename = tempnam(sys_get_temp_dir(), 'vmmsgqueue');
  $msg_id = msg_get_queue(ftok($filename, 'a'), 0600);

  try {
    send($msg_id, "hello");
    receive($msg_id);

    send($msg_id, "");
    receive($msg_id);

    send($msg_id, str_repeat("*", 1024));
    receive($msg_id);
  } finally {
    msg_remove_queue($msg_id);
    unlink($filename);
  }
}
