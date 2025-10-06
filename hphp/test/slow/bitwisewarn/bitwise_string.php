<?hh

function VS($a, $b) :mixed{
  if ($a === $b) var_dump("ok");
  else {
    var_dump($a);
    var_dump(debug_backtrace());
  }
}
<<__EntryPoint>> function main(): void {
  $s = "\x50\x51" | "\x51\x51";          VS($s, "\x51\x51");
  $s = "\x50\x51" & "\x51\x51";          VS($s, "\x50\x51");
  $s = "\x50\x51" ^ "\x51\x51";          VS($s, "\x01\x00");
  $s = \HH\str_bitwise_xor("\x50\x51", "\x51\x51"); VS($s, "\x01\x00");
  $s = "\x50\x51"; $s |= "\x51\x51";     VS($s, "\x51\x51");
  $s = "\x50\x51"; $s &= "\x51\x51";     VS($s, "\x50\x51");
  $s = "\x50\x51"; $s ^= "\x51\x51";     VS($s, "\x01\x00");
  $s = "\x50\x51"; $s = ~$s;             VS($s, "\xAF\xAE");
}
