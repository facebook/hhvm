<?php
$Foo = array('val1', 'val2', 'val3');
end($Foo);
echo key($Foo),"\n";
$MagicInternalPointerResetter = $Foo;
echo key($Foo),"\n";
?>