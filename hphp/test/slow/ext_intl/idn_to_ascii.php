<?php

// Php doesn't support \u escapes.
function u($x) { return json_decode("\"" . $x . "\""); }

var_dump(idn_to_ascii("www.m\xc3\xa5nsjonasson.se") ===
    "www.xn--mnsjonasson-x8a.se");
var_dump(idn_to_ascii("www.facebook.com"));
var_dump(idn_to_ascii("www.xn--m\xc3\xa5nsjonasson.se") === false);
var_dump(idn_to_ascii("www.12345678901234567890123456789".
                  "012345678901234m\xc3\xa5nsjonasson.se")
                  ===
    "www.xn--123456789012345678901234567890123456789".
    "01234mnsjonasson-5we.se");
var_dump(idn_to_ascii("www.12345678901234567890123456789".
                  "0123456789012345m\xc3\xa5nsjonasson.se") ===
    false);
var_dump(idn_to_ascii(u('\u1937ai\u18ed-\u18f0.tk'), 1) ===
  "xn--ai--youq53b.tk");
var_dump(idn_to_ascii(u('\u1937ai\u18ed-\u18f0.tk'), 0) ===
  false);
