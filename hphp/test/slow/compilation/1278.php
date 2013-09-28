<?php

class T {
 function __toString() {
 return 123;
}
}
 $obj = new T();
 var_dump($obj);
