<?php

var_dump(mb_http_input());
var_dump(mb_http_output());
var_dump(mb_language());
var_dump(mb_preferred_mime_name("sjis-win"));

mb_regex_encoding("UTF-8");
var_dump(mb_regex_encoding());

var_dump(mb_regex_set_options());
var_dump(mb_regex_set_options("pz"));
var_dump(mb_regex_set_options());
