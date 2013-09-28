<?php

$a = 123;
echo $a ? @mysql_data_seek(null, null) : false;
