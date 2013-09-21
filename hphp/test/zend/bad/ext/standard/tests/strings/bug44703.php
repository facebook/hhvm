<?php

var_dump(htmlspecialchars(b"<a href='test'>Test</a>", ENT_COMPAT, 1));
var_dump(htmlspecialchars(b"<a href='test'>Test</a>", ENT_COMPAT, 12));
var_dump(htmlspecialchars(b"<a href='test'>Test</a>", ENT_COMPAT, 125));
var_dump(htmlspecialchars(b"<a href='test'>Test</a>", ENT_COMPAT, 1252));
var_dump(htmlspecialchars(b"<a href='test'>Test</a>", ENT_COMPAT, 12526));

var_dump(htmlspecialchars(b"<>", ENT_COMPAT, 866));
var_dump(htmlspecialchars(b"<>", ENT_COMPAT, 8666));

var_dump(htmlspecialchars(b"<>", ENT_COMPAT, NULL));


var_dump(htmlspecialchars(b"<>", ENT_COMPAT, 'SJIS'));
var_dump(htmlspecialchars(b"<>", ENT_COMPAT, 'SjiS'));

var_dump(htmlspecialchars(b"<>", ENT_COMPAT, str_repeat('a', 100)));

?>