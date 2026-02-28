<?hh
function foo () :mixed{
    for($i = 0; $i < 5; $i++) {
        do {
            try {
                try {
                } finally {
                }
            } catch (Exception $e) {
            } finally {
              continue;
            }
        } while (0);
    }
}
<<__EntryPoint>> function main(): void {
foo();
}
