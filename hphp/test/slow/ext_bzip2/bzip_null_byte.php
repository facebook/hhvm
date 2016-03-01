<?php

$file = '/etc/passwd'.chr(0).'asdf';

var_dump(bzopen($file, 'r'));
