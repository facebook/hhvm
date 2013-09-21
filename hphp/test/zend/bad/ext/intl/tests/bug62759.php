<?php
var_dump(substr('deja', 1, -4));
var_dump(substr('deja', -1, 0));
var_dump(grapheme_substr('deja', 1, -4));
var_dump(intl_get_error_message());
var_dump(grapheme_substr('deja', -1, 0));
var_dump(grapheme_substr('déjà', 1, -4));
var_dump(intl_get_error_message());
var_dump(grapheme_substr('déjà', -1, 0));
?>