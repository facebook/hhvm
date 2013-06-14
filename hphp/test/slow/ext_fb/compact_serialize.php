<?php

function fb_cs_test($v) {
  echo "====\n";

  $ret = null;
  $v_ = $v;
  $s_ = fb_compact_serialize($v_);
  var_dump(is_string($s_));
  $ss_ = $s_;
  var_dump(!empty($ss_));
  /* check high bit of first character always set */
  var_dump(preg_match("/^[\x80-\xff]/", $ss_));
  var_dump(fb_compact_unserialize($s_, $ret) === $v_);
  var_dump($ret === true);
  $ret = null;
  var_dump(fb_unserialize($s_, $ret) === $v_);
  var_dump($ret === true);
}

function main() {
  fb_cs_test(null);
  fb_cs_test(true);
  fb_cs_test(false);
  fb_cs_test(1234.5678);
  fb_cs_test("");
  fb_cs_test("a");
  fb_cs_test("\0");
  fb_cs_test("\0 a");
  fb_cs_test("0123012301230123");
  fb_cs_test("0123012301230123a");
  fb_cs_test("012301230123012");
  fb_cs_test(array());
  fb_cs_test(array(12345));
  fb_cs_test(array(12345,"abc",0.1234));
  fb_cs_test(array(1 => 12345));
  fb_cs_test(array(1 => 12345, "a" => 123124, "sdf" => 0.1234));
  fb_cs_test(array(array("a")));
  fb_cs_test(array(1, array("a")));
  fb_cs_test(array(array("a"), 1));
  fb_cs_test(array(array("a"), array(1)));

  // Test skips
  fb_cs_test(array(0 => "a", 1 => "b", 3 => "c"));
  fb_cs_test(array(1 => "a", 2 => "b", 3 => "c"));
  fb_cs_test(array(0 => "a", 2 => "b", 3 => "c"));
  fb_cs_test(array(3 => "a"));
  // Test for overflow (1ull << 63) - 1
  fb_cs_test(array(9223372036854775807, 'a'));

  // Test each power of two, +/- 1 and the negatives of them
  // Test a single number and packed inside an array
  for ($i = 0; $i < 64; ++$i) {
    $n = 1 << $i;
    fb_cs_test($n);    fb_cs_test(array($n));
    fb_cs_test($n-1);  fb_cs_test(array($n-1));
    fb_cs_test($n+1);  fb_cs_test(array($n+1));
    fb_cs_test(-$n);   fb_cs_test(array(-$n));
    fb_cs_test(-$n-1); fb_cs_test(array(-$n-1));
    fb_cs_test(-$n+1); fb_cs_test(array(-$n+1));
  }

  echo "====\n";

  // Test vector code (PHP can't create those, but they might come form
  // C++ code in serialized strings)
  $s = "\xfe\x01\x02\x03\xfc";  // VECTOR, 1, 2, 3, STOP
  $ret = null;
  var_dump(fb_compact_unserialize($s, $ret));
}

main();
