<?php

$a = array('<foo>',"'bar'",'"baz"','&blong&');

echo "Normal: ", json_encode($a), "\n";
echo "Tags: ",   json_encode($a,JSON_HEX_TAG), "\n";
echo "Apos: ",   json_encode($a,JSON_HEX_APOS), "\n";
echo "Quot: ",   json_encode($a,JSON_HEX_QUOT), "\n";
echo "Amp: ",    json_encode($a,JSON_HEX_AMP), "\n";
echo "All: ",    json_encode($a,JSON_HEX_TAG|JSON_HEX_APOS|JSON_HEX_QUOT|JSON_HEX_AMP), "\n";
?>