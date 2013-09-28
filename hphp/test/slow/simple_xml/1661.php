<?php

$a = simplexml_load_string('<?xml version="1.0" encoding="utf-8"?><node><subnode><subsubnode attr1="value1">test</subsubnode></subnode></node>');
var_dump((string)($a->subnode->subsubnode['attr1']));
