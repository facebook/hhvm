<?php
if (base64_decode(b"dGVzdA==") == base64_decode(b"dGVzdA==CRAP")) {
    echo "Same octect data - Signature Valid\n";
} else {
    echo "Invalid Signature\n";
}

$in = base64_encode(b"foo") . b'==' . base64_encode(b"bar");
var_dump($in, base64_decode($in));

?>