<?php

function VS($x, $y) {
  var_dump($x === $y);
  if ($x !== $y) { echo "Failed: $y\n"; echo "Got: $x\n";
                   var_dump(debug_backtrace()); }
}

function VUNPACK($fmt, $inp, $exp) {
  $a = unpack($fmt, $inp);
  VS(isset($a[1]), true);
  VS($a[1], (int)$exp);
}

function test_unpack() {
  $iFF = str_repeat("\xFF", 4);
  $le32_FF = "\xFF\x00\x00\x00";
  $be32_FF = "\x00\x00\x00\xFF";
  $le16_FF = "\xFF\x00";
  $be16_FF = "\x00\xFF";

  // HPHP, unlike PHP, truncates overflowing ints
  VUNPACK("I", $iFF, 0xFFFFFFFF);

  VUNPACK("i", $iFF, -1);

  // LlNV test 32-bit ints specifically
  VUNPACK("L", $iFF, 0xFFFFFFFF);
  VUNPACK("l", $iFF, -1);

  VUNPACK("N", $be32_FF, 0xFF);
  VUNPACK("V", $le32_FF, 0xFF);
  VUNPACK("V", $be32_FF, 0xFF000000);

  VUNPACK("L", $le32_FF, 0xFF);

  // Ssnv test 16-bit shorts
  VUNPACK("S", $iFF, 0xFFFF);
  VUNPACK("s", $iFF, -1);

  VUNPACK("n", $be16_FF, 0xFF);
  VUNPACK("v", $le16_FF, 0xFF);
  VUNPACK("v", $be16_FF, 0xFF00);

  VUNPACK("S", $le16_FF, 0xFF);
}

test_unpack();
