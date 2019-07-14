<?hh
<<__EntryPoint>> function foo (): void {
   try {
   } finally {
      goto label;
   }
label:
   return 1;
}
