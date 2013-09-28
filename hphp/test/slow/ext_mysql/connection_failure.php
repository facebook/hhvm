<?php

// This should fail. Even if it succeeds, the subsequent calls should not crash
// which is what this test is trying to verify.
@mysql_pconnect('localhost', 'notreal', 'notreal');
mysql_set_charset('latin1');
mysql_error();
mysql_errno();
mysql_warning_count();

echo "Done\n";
