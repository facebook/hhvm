<?php

try {
 try {
 throw new Exception('test');
}
 catch (InvalidArgumentException $e) {
}
 }
 catch (Exception $e) {
 print 'ok';
}
