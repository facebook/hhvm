<?php

/*
 * Test IDN functions (procedural only)
 */

echo idn_to_ascii("t\xC3\xA4st.de")."\n";
echo urlencode(idn_to_utf8('xn--tst-qla.de'))."\n";

?>