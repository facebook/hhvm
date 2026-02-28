<?hh

function test($n) :mixed{
  return mb_send_mail("A\x00".str_repeat('b', $n), '', '');
}

<<__EntryPoint>> function main(): void {
  var_dump(test(32));
}
