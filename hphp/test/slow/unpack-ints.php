<?hh

<<__EntryPoint>>
function main() {
  $ints = vec[
    PHP_INT_MIN,
    PHP_INT_MAX,
    -1,
    0,
    1,
    0xfedc,
    0xfedcba,
    0xfedcba98,
    0xfedcba9876,
    0xfedcba987654,
    0xfedcba98765432,
    0xfedcba987654321,
    0x01,
    0x0123,
    0x012345,
    0x01234567,
    0x0123456789,
    0x0123456789ab,
    0x0123456789abcd,
    0x0123456789abcdef
  ];
  $format_flags = vec['c', 'C', 's', 'S', 'n', 'v', 'i', 'I', 'l', 'L', 'N', 'V', 'q', 'Q', 'J', 'P'];

  foreach ($format_flags as $flag) {
    echo "Format: $flag\n";
    foreach ($ints as $i) {
      $var = pack($flag, $i);
      echo $i .': '. unpack("H*", $var)[1] .', '. unpack($flag, $var)[1]."\n";
    }
  }
}
