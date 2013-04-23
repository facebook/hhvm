<?php

$envelope = array('return_path' => 'John Doe <john@example.com>',
                  'from'        => 'John Doe <john@example.com>',
                  'reply_to'    => 'John Doe <john@example.com>',
                  'to'          => 'John Doe <john@example.com>',
                  'cc'          => 'John Doe <john@example.com>',
                  'bcc'         => 'John Doe <john@example.com>',
);

var_dump($envelope);
imap_mail_compose($envelope, array(1 => array()));
var_dump($envelope);

?>