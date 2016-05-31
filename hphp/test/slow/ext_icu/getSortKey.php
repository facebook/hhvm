<?php
$coll = new Collator('root');
var_dump(substr_count($coll->getSortKey('Hello'), "\0"));
