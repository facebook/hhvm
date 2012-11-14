--TEST--
Script Fastpath
--FILE--
<?php
$foo = xhp_preprocess_code('<?php
$foo = <script>bar</script>;');
echo isset($foo['new_code']);
--EXPECT--
1
