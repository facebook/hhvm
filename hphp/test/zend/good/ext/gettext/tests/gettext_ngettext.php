<?php // $Id$
chdir(dirname(__FILE__));
setlocale(LC_ALL, 'en_US.UTF-8');
bindtextdomain('dngettextTest', './locale');
textdomain('dngettextTest');
var_dump(ngettext('item', 'items', 1));
var_dump(ngettext('item', 'items', 2));
?>