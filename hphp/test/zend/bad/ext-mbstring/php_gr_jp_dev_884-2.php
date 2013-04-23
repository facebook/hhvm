<?php
var_dump(mb_ereg_replace("C?$", "Z", "ABC"));
var_dump(ereg_replace("C?$", "Z", "ABC"));
var_dump(mb_ereg_replace("C*$", "Z", "ABC"));
var_dump(ereg_replace("C*$", "Z", "ABC"));
?>