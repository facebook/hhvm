<?php

function sendToHarness($chr) {
  $fname = '/tmp/hphpd_test_fifo.'.posix_getppid();
  error_log('sending flag: '.$chr);
  $fp = fopen($fname, 'w');
  if (!$fp) {
    throw new TestFailure("failed to open test fifo");
  }
  if (!fwrite($fp, $chr, 1)) {
    throw new TestFailure("failed to write test fifo");
  }
  fflush($fp);
  fclose($fp);
}
