<?php

// "abc", ordinarily 'b' would be \142, but we'll deliberately overflow the value by \400
echo "\141\542\143\n";
