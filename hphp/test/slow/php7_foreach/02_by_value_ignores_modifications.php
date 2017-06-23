<?php
$a = [1,2,3]; $b = &$a; foreach($a as $v) {echo "$v\n"; unset($a[1]);}
