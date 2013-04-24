<?php
//line 2
//line 3
//line 4
//line 5
$s = new SplFileObject(__FILE__);
$s->seek(2);

echo $s->current('foo');
?>