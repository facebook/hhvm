<?php
namespace foo {
echo "hi\n";
}
__halt_compiler();
namespace unprocessed {
echo "should not echo\n";
}
echo "===DONE===\n";
