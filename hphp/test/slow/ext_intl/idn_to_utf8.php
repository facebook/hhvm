<?php

var_dump(idn_to_utf8("www.xn--mnsjonasson-x8a.se") ===
    "www.m\xc3\xa5nsjonasson.se");
var_dump(idn_to_utf8("www.facebook.com"));
var_dump(idn_to_utf8("www.xn--12345678901234567890123456789".
                 "012345678901234mnsjonasson-5we.se")
  ===
    "www.12345678901234567890123456789".
    "012345678901234m\xc3\xa5nsjonasson.se");
