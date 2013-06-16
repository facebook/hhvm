<?php

mb_internal_encoding("ISO-8859-1");
var_dump(mb_encode_mimeheader("Subject: Pr\xDC"."fung Pr\xDC"."fung",
                          "UTF-8", "B") ===
   "Subject: =?UTF-8?B?UHLDnGZ1bmcgUHLDnGZ1bmc=?=");
mb_internal_encoding("UTF-8");
