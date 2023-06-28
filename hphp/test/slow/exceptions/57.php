<?hh

class E extends Exception {
 function __toString():mixed{
 return 'E';
}
}
 class F extends E {
 function __toString()[] :mixed{
 return 'F';
}
}

<<__EntryPoint>>
function main_57() :mixed{
try {
 throw new F();
 }
 catch (E $e) {
 print $e;
}
}
