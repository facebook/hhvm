<?php

<<__EntryPoint>>
function main_grapheme_strlen_invalid() {
var_dump(grapheme_strlen("\xE9 invalid UTF-8"));
}
