<?php
chdir(dirname(__FILE__));
setlocale(LC_ALL, 'en_US.UTF-8');
bindtextdomain('dngettextTest', './locale');

var_dump(dngettext('dngettextTest', 'item', 'items'));