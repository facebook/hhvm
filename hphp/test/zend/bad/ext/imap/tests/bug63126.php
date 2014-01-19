<?php
$tests = array(
    'Array'  => array('DISABLE_AUTHENTICATOR' => array('GSSAPI','NTLM')),
    'String' => array('DISABLE_AUTHENTICATOR' => 'GSSAPI'),
);
require_once(dirname(__FILE__).'/imap_include.inc');
foreach ($tests as $name => $testparams) {
    echo "Test for $name\n";
    $in = imap_open($default_mailbox, $username, $password, OP_HALFOPEN, 1, $testparams);
    if ($in) {
        if (is_array($errors = imap_errors())) {
            foreach ($errors as $err) {
                if (strstr($err, 'GSSAPI') || strstr($err, 'Kerberos')) {
                    echo "$err\n";
                }
            }
        }
    } else {
        echo "Can't connect\n";
    }
}
echo "Done\n";
?>