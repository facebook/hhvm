<?hh
<<__EntryPoint>>
function main_entry(): void {
    try {
    } finally {
      try {
         goto foo;
      }
      finally {}
      foo:
    }
}
