<?php

function iterate($xml)
{
    print_r($xml);
    foreach ($xml->item as $item) {
        echo "This code will crash!";
    }
}

$xmlstr = "<xml><item>Item 1</item><item>Item 2</item></xml>";
$xml = simplexml_load_string($xmlstr);
iterate($xml->unknown);

echo "Done\n";
?>