<?hh
function foo () :mixed{
    do {
        try {
            try {
            } finally {
                break;
            }
        } catch (Exception $e) {
        } finally {
        }
    } while (0);
}
<<__EntryPoint>> function main(): void {
foo();
}
