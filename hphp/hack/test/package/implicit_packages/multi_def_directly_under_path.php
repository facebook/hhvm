//// prototypes/many_defs.php
<?hh
// Five top-level definitions in one badly-placed file; exactly one error.
const int MANY_C = 1;

function many_f(): void {}

class ManyC {}

interface ManyI {}

type ManyT = int;
