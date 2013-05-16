<?php
var_dump(html_entity_decode("&amp;lt;", ENT_COMPAT, 'koi8-r'));
var_dump(html_entity_decode("&amp;#38;", ENT_COMPAT, 'koi8-r'));
var_dump(html_entity_decode("&amp;#38;lt;", ENT_COMPAT, 'koi8-r'));
?>