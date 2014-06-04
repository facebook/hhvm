<?php

foreach (array(null, "", false, 0) as $type) {
    $xml = simplexml_load_string('<root />');
    $xml->addChild("a", "b", $type);
    var_dump(str_replace(PHP_EOL, "", $xml->a->asXML()));
}
