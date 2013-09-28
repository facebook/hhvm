<?php

class E extends Exception {
 function __toString(){
 return 'E';
}
}
 class F extends E {
 function __toString() {
 return 'F';
}
}
try {
 throw new F();
 }
 catch (E $e) {
 print $e;
}
