<?php

$file = '/etc/passwd'.chr(0).'asdf';

var_dump(imap_open($file, 'user', 'pass'));
