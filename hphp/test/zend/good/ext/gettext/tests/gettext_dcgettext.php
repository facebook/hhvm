<?php
chdir(dirname(__FILE__));
setlocale(LC_MESSAGES, 'en_US.UTF-8');
setlocale(LC_ALL, 'en_US.UTF-8');
bindtextdomain('dngettextTest', './locale');

var_dump(dcgettext('dngettextTest', 'item', LC_CTYPE));
var_dump(dcgettext('dngettextTest', 'item', LC_MESSAGES));