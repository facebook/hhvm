<?php
$a = [1,2]; foreach($a as &$v) {echo "$v\n"; $a[2]=3;}
