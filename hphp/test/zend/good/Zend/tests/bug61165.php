<?php

$handler = NULL;
class T {
    public $_this;

    public function __toString() {
		global $handler;
	    $handler = $this;
        $this->_this = $this; // <-- uncoment this
        return 'A';
    }
}

$t = new T;
for ($i = 0; $i < 3; $i++) {
    strip_tags($t);
	strip_tags(new T);
}
var_dump($handler);