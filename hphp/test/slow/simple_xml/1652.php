<?php

$doc = simplexml_load_String('<?xml version="1.0"?><lists><list path="svn+ssh"><entry kind="dir"></entry><entry kind="file"></entry></list></lists>');
 foreach ($doc->list[0]->entry as $r) {
 var_dump((array)$r->attributes());
}
