<?php

$a = new SimpleXMLElement('<?xml version="1.0" encoding="utf-8"?><node><subnode><subsubnode><sssnode>test</sssnode></subsubnode></subnode></node>');
var_dump((string)($a->subnode->subsubnode->sssnode));
