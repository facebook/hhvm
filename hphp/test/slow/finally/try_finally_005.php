<?hh

<<__EntryPoint>>
function foo () {
   try {
   } finally {
      goto label;
   }
label:
   return 1;
}
