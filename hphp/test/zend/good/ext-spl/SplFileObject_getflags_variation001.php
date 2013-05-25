<?php

$fo = new SplFileObject(__FILE__);

$fo->setFlags(SplFileObject::DROP_NEW_LINE);
var_dump($fo->getFlags());

$fo->setFlags(SplFileObject::READ_AHEAD);
var_dump($fo->getFlags());

$fo->setFlags(SplFileObject::SKIP_EMPTY);
var_dump($fo->getFlags());

$fo->setFlags(SplFileObject::READ_CSV);
var_dump($fo->getFlags());

?>