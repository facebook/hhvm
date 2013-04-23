<?php
var_dump(json_encode("latin 1234 -/    russian мама мыла раму  specialchars \x02   \x08 \n   U+1D11E >𝄞<"));
var_dump(json_encode("latin 1234 -/    russian мама мыла раму  specialchars \x02   \x08 \n   U+1D11E >𝄞<", JSON_UNESCAPED_UNICODE));
var_dump(json_encode("ab\xE0"));
var_dump(json_encode("ab\xE0", JSON_UNESCAPED_UNICODE));
?>