<?php

$tmpfname = tempnam("/tmp", "FOO");

class foo {
    public function __toString()
    {
        return 'hello';
    }
}
$object = new foo;
file_put_contents($tmpfname, $object);
echo file_get_contents($tmpfname);
