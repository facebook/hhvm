<?hh

//////////////////////////////////////////////////////////////////////

function VD(mixed $i, bool $hexdump=false): void {
  if (!$hexdump) {
    var_dump($i);
    return;
  }
  printf("0x%0x\n", $i);
}

<<__EntryPoint>>
function main(): void {
  $packed = str_repeat(chr(0), 16);
  var_dump(inet_ntop($packed));
  var_dump(inet_ntop_nullable($packed));
  var_dump(inet_ntop_folly($packed));

  $packed = str_repeat(chr(0), 15).chr(1);
  var_dump(inet_ntop($packed));
  var_dump(inet_ntop_nullable($packed));
  var_dump(inet_ntop_folly($packed));

  $packed = chr(127).chr(0).chr(0).chr(1);
  var_dump(inet_ntop($packed));
  var_dump(inet_ntop_nullable($packed));
  var_dump(inet_ntop_folly($packed));

  var_dump(inet_ntop(inet_pton("127.0.0.1")));
  var_dump(inet_ntop_nullable(inet_pton("127.0.0.1")));
  var_dump(inet_ntop_folly(inet_pton("127.0.0.1")));

  $packed = str_repeat(chr(0), 15).chr(1);
  var_dump(inet_ntop($packed));
  var_dump(inet_ntop_nullable($packed));
  var_dump(inet_ntop_folly($packed));
  var_dump(inet_ntop(inet_pton("::1")));
  var_dump(inet_ntop_nullable(inet_pton("::1")));
  var_dump(inet_ntop_folly(inet_pton("::1")));

  $oneone = __hhvm_intrinsics\launder_value("1::1");
  var_dump(inet_ntop(inet_pton($oneone)));
  var_dump(inet_ntop_nullable(inet_pton($oneone)));
  var_dump(inet_ntop_folly(inet_pton($oneone)));
  var_dump(inet_ntop(str_repeat(chr(0), 3)) === false);
  var_dump(inet_ntop_nullable(str_repeat(chr(0), 3)) is null);
  var_dump(inet_ntop_folly(str_repeat(chr(0), 3)) is null);

}
