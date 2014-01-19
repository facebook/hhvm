<?php

$tests = array(
    "&#0;", //C0
    "&#1;",
    "&#x09;",
    "&#x0A;",
    "&#x0B;",
    "&#x0C;",
    "&#x0D;", //note that HTML5 is unique in that it forbids this entity, but allows a literal U+0D
    "&#x0E;",
    "&#x1F;",
    "&#x20;", //allowed always
    "&#x7F;", //DEL
    "&#x80;", //C1
    "&#x9F;",
    "&#xA0;", //allowed always
    "&#xD7FF;", //surrogates
    "&#xD800;",
    "&#xDFFF;",
    "&#xE000;", //allowed always
    "&#xFFFE;", //nonchar
    "&#xFFFF;",
    "&#xFDCF;", //allowed always
    "&#xFDD0;", //nonchar
    "&#xFDEF;",
    "&#xFDF0;", //allowed always
    "&#x2FFFE;", //nonchar
    "&#x2FFFF;",
);

echo "*** HTML 4.01  ***\n";

foreach ($tests as $t) {
    $dec = html_entity_decode($t, ENT_QUOTES | ENT_HTML401, "UTF-8");
    if ($t == $dec) {
        echo "$t\tNOT DECODED\n";
    } else {
        echo "$t\tDECODED\n";
    }
}

echo "\n*** XHTML 1.0  ***\n";

foreach ($tests as $t) {
    $dec = html_entity_decode($t, ENT_QUOTES | ENT_XHTML, "UTF-8");
    if ($t == $dec) {
        echo "$t\tNOT DECODED\n";
    } else {
        echo "$t\tDECODED\n";
    }
}

echo "\n*** HTML5  ***\n";

foreach ($tests as $t) {
    $dec = html_entity_decode($t, ENT_QUOTES | ENT_HTML5, "UTF-8");
    if ($t == $dec) {
        echo "$t\tNOT DECODED\n";
    } else {
        echo "$t\tDECODED\n";
    }
}

echo "\n*** XML 1.0  ***\n";

foreach ($tests as $t) {
    $dec = html_entity_decode($t, ENT_QUOTES | ENT_XML1, "UTF-8");
    if ($t == $dec) {
        echo "$t\tNOT DECODED\n";
    } else {
        echo "$t\tDECODED\n";
    }
}

echo "\nDone.\n";