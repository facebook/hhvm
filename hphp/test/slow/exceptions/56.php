<?php

class E extends Exception {
}
 class F extends E {
}
try {
 throw new F();
 }
 catch (E $e) {
 print 'ok';
}
