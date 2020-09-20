<?hh


<<__EntryPoint>>
function main_54() {
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
}
