<?hh

class E extends Exception {
}

 <<__EntryPoint>>
function main_55() {
try {
 throw new E();
 }
 catch (E $e) {
 print 'ok';
}
}
