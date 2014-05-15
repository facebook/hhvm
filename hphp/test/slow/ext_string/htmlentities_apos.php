<?php

var_dump(htmlentities('\'', ENT_QUOTES));
var_dump(htmlentities('\'', ENT_QUOTES | ENT_XML1));
var_dump(htmlentities('\'', ENT_QUOTES | ENT_XHTML));
var_dump(htmlentities('\'', ENT_QUOTES | ENT_HTML401));
var_dump(htmlentities('\'', ENT_QUOTES | ENT_HTML5));
