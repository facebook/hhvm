<?php
$overflown = str_repeat('C', 8476509);
$msgid     = "msgid";
$domain    = "domain";
$category  = "cat";

var_dump(bindtextdomain($overflown, 'path'));

var_dump(dngettext($overflown, $msgid, $msgid, 1));
var_dump(dngettext($domain, $overflown, $msgid, 1));
var_dump(dngettext($domain, $msgid, $overflown, 1));

var_dump(gettext($overflown));

var_dump(ngettext($overflown, $msgid, -1));
var_dump(ngettext($msgid, $overflown, -1));

var_dump(dcgettext($overflown, $msgid, -1));
var_dump(dcgettext($domain, $overflown, -1));

var_dump(dcngettext($overflown, $msgid, $msgid, -1, -1));
var_dump(dcngettext($domain, $overflown, $msgid, -1, -1));
var_dump(dcngettext($domain, $msgid, $overflown, -1, -1));

var_dump(dgettext($overflown, $msgid));
var_dump(dgettext($domain, $overflown));

var_dump(textdomain($overflown));
?>
==DONE==