<?php

$s = "\xc3\xa8\xc3\xa9\xc3\xa6\xc3\x83\xc2\xb3";
var_dump($s);
var_dump(htmlspecialchars($s, ENT_COMPAT, 'UTF-8'));
var_dump(htmlspecialchars($s, ENT_COMPAT | ENT_IGNORE, 'UTF-8'));
var_dump(htmlspecialchars($s, ENT_COMPAT | ENT_SUBSTITUTE, 'UTF-8'));

var_dump(htmlspecialchars("a\x80b"));
var_dump(htmlspecialchars("a\x80b", ENT_IGNORE));
var_dump(htmlspecialchars("a\x80b", ENT_SUBSTITUTE));
var_dump(htmlspecialchars("a\x80b", ENT_IGNORE | ENT_SUBSTITUTE));
