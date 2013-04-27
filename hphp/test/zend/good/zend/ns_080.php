<?php
namespace foo {
echo "hi\n";
}
__HALT_COMPILER();
namespace unprocessed {
echo "should not echo\n";
}
?>
===DONE===