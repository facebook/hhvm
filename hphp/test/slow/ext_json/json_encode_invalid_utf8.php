<?php

function dump_json_encode($charstream) {
  print "input " . str_replace('%', '\\x', rawurlencode($charstream)) . "\n";
  print "loose " ;
  var_dump(json_encode($charstream, JSON_FB_LOOSE));
  print "strict ";
  var_dump(json_encode($charstream, 0));
  print "\n";
}

function test_json_encode() {
// Invalid and valid byte sequences
  dump_json_encode("\xE2\x82\xAC\xC2\xA2");
  dump_json_encode("\xE2\x82\xAC\xC2");
  dump_json_encode("\xE2\x82\xC2\xA2");
  dump_json_encode("\xE2\x82\xC2");
  dump_json_encode("\xE2\x82". '"');

  print ("\n\n2-byte UTF\n");
// 2-byte UTF8
// In the middle
  dump_json_encode("123" . "\xC2". "456");
  dump_json_encode("123" . "\xC2\xA2". "456");
// At the end
  dump_json_encode("123" . "\xC2");
  dump_json_encode("123" . "\xC2\xA2");
// At the beginning
  dump_json_encode("\xC2" . "456");
  dump_json_encode("\xC2\xA2" . "456");
// On its own
  dump_json_encode("\xC2");
  dump_json_encode("\xC2\xA2");

  print ("\n\n3-byte UTF\n");
// 3-byte UTF8
// In the middle
  dump_json_encode("123" . "\xE2" . "456");
  dump_json_encode("123" . "\xE2\x82" . "456");
  dump_json_encode("123" . "\xE2\x82\xAC" . "456");
// At the end
  dump_json_encode("123" . "\xE2");
  dump_json_encode("123" . "\xE2\x82");
  dump_json_encode("123" . "\xE2\x82\xAC");
// At the beginning
  dump_json_encode("\xE2". "456");
  dump_json_encode("\xE2\x82" . "456");
  dump_json_encode("\xE2\x82\xAC" . "456");
// On its own
  dump_json_encode("\xE2");
  dump_json_encode("\xE2\x82");
  dump_json_encode("\xE2\x82\xAC");

  print ("\n\n4-byte UTF\n");
// 4-byte UTF8
// In the middle
  dump_json_encode("123". "\xF0" . "456");
  dump_json_encode("123" . "\xF0\x90" . "456");
  dump_json_encode("123" . "\xF0\x90\x80" . "456");
  dump_json_encode("123" . "\xF0\x90\x80\x80" . "456");
// At the end
  dump_json_encode("123" . "\xF0");
  dump_json_encode("123" . "\xF0\x90");
  dump_json_encode("123" . "\xF0\x90\x80");
  dump_json_encode("123" . "\xF0\x90\x80\x80");

// At the beginning
  dump_json_encode("\xF0" . "456");
  dump_json_encode("\xF0\x90" . "456");
  dump_json_encode("\xF0\x90\x80" . "456");
  dump_json_encode("\xF0\x90\x80\x80". "456");
// On its own
  dump_json_encode("\xF0");
  dump_json_encode("\xF0\x90");
  dump_json_encode("\xF0\x90\x80");
  dump_json_encode("\xF0\x90\x80\x80");


  print ("\n\nMisc\n");
// Misc.
  dump_json_encode("123\xE0456");
  dump_json_encode("\xE0\xE0");
  dump_json_encode("12");
  dump_json_encode(
      '<a href="/foo' . chr(0xC0). '" title=" onmouseover=alert(1) //">');
}

test_json_encode();
