<?php

mb_internal_encoding("ISO-8859-1");
var_dump(mb_decode_mimeheader("Subject: =?UTF-8?B?UHLDnGZ1bmcgUHLDnGZ1bmc=?=\n") ===
   "Subject: Pr\xDC"."fung Pr\xDC"."fung");
mb_internal_encoding("UTF-8");
