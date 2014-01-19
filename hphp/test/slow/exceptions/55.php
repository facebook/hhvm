<?php

class E extends Exception {
}
 try {
 throw new E();
 }
 catch (E $e) {
 print 'ok';
}
