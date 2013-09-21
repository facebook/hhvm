<?php

quoted_printable_encode(str_repeat("\xf4", 1000)); 
quoted_printable_encode(str_repeat("\xf4", 100000)); 

echo "Done\n";
?>