<?php

$date = date_create_immutable();
var_dump(dom_import_simplexml($date));
var_dump(simplexml_import_dom($date));
