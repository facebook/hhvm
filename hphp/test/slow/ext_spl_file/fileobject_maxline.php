<?php

$fo = new SplFileObject(__FILE__);
$fo->setMaxLineLen(3);
echo $fo->getCurrentLine();
