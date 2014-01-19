<?php
chdir(dirname(__FILE__));
setlocale(LC_MESSAGES, 'en_US.UTF-8');
setlocale(LC_ALL, 'en_US.UTF-8');
bindtextdomain('dgettextTest', './locale');
bindtextdomain('dgettextTest_switch', './locale');
textdomain('dgettextTest');

var_dump(gettext('item'));
var_dump(dgettext('dgettextTest_switch', 'item'));
var_dump(gettext('item'));
?>