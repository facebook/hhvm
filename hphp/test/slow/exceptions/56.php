<?hh

class E extends Exception {
}
 class F extends E {
}

<<__EntryPoint>>
function main_56() {
try {
 throw new F();
 }
 catch (E $e) {
 print 'ok';
}
}
