<?php

ZendGoodZendTestsBug61165::$handler = NULL;
class T {
    public $_this;

    public function __toString() {

	    ZendGoodZendTestsBug61165::$handler = $this;
        $this->_this = $this; // <-- uncoment this
        return 'A';
    }
}

$t = new T;
for ($i = 0; $i < 3; $i++) {
    strip_tags($t);
	strip_tags(new T);
}
var_dump(ZendGoodZendTestsBug61165::$handler);

abstract final class ZendGoodZendTestsBug61165 {
  public static $handler;
}
