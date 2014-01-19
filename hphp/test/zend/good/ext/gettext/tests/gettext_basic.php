<?php // $Id$

chdir(dirname(__FILE__));
setlocale(LC_ALL, 'fi_FI');
bindtextdomain ("messages", "./locale");
textdomain ("messages");
echo gettext("Basic test"), "\n";
echo _("Basic test"), "\n";

?>