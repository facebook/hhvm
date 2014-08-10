<?php
$tests = array(
    "&quot;", "&#x22;", "&#34;",
    "&apos;", "&#39;", "&#x27;",
    "&amp;", "&#x26;", "&lt;",
    "&gt;", "&#x3C;", "&#60;",
    "&lt;", "&#x3E;", "&#62;",
    "&#63;"
);

echo "*** HTML 4.01/ENT_QUOTES  ***\n";

foreach ($tests as $t) {
    $dec = htmlspecialchars_decode($t, ENT_QUOTES | ENT_HTML401);
    if ($t == $dec) {
        echo "$t\tNOT DECODED\n";
    } else {
        echo "$t\tDECODED\n";
    }
}

echo "\n*** XHTML 1.0/ENT_QUOTES  ***\n";

foreach ($tests as $t) {
    $dec = htmlspecialchars_decode($t, ENT_QUOTES | ENT_XHTML);
    if ($t == $dec) {
        echo "$t\tNOT DECODED\n";
    } else {
        echo "$t\tDECODED\n";
    }
}

echo "\n*** HTML5/ENT_QUOTES  ***\n";

foreach ($tests as $t) {
    $dec = htmlspecialchars_decode($t, ENT_QUOTES | ENT_HTML5);
    if ($t == $dec) {
        echo "$t\tNOT DECODED\n";
    } else {
        echo "$t\tDECODED\n";
    }
}

echo "\n*** XML 1.0/ENT_QUOTES  ***\n";

foreach ($tests as $t) {
    $dec = htmlspecialchars_decode($t, ENT_QUOTES | ENT_XML1);
    if ($t == $dec) {
        echo "$t\tNOT DECODED\n";
    } else {
        echo "$t\tDECODED\n";
    }
}

echo "\n*** HTML5/ENT_NOQUOTES  ***\n";

foreach ($tests as $t) {
    $dec = htmlspecialchars_decode($t, ENT_NOQUOTES | ENT_HTML5);
    if ($t == $dec) {
        echo "$t\tNOT DECODED\n";
    } else {
        echo "$t\tDECODED\n";
    }
}

echo "\n*** HTML5/ENT_COMPAT  ***\n";

foreach ($tests as $t) {
    $dec = htmlspecialchars_decode($t, ENT_COMPAT | ENT_HTML5);
    if ($t == $dec) {
        echo "$t\tNOT DECODED\n";
    } else {
        echo "$t\tDECODED\n";
    }
}


echo "\nDone.\n";
?>
