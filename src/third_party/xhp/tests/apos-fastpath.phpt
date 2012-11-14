--TEST--
Apostrophe Fastpath
--FILE--
<?php
$foo = xhp_preprocess_code('<?php
$foo = <div>Her\'s</div>;foo(\'\');');
echo isset($foo['new_code']);
--EXPECT--
1
