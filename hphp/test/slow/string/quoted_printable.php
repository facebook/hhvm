<?php
echo strlen(quoted_printable_encode(str_repeat("\xf4", 1000000000)));
