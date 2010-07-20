--TEST--
Heredoc Fastpath
--FILE--
<?php
$foo = <<<EOF
?>
EOF;

// 5.2 doesn't support nowdoc so we can't put this in the test or else it will
// fail. Instead we use xhp_preprocess_code to make sure nowdoc and binary
// nowdocs can parse.
$d = '$';
$code = <<<EOF
<?php
{$d}foo = <<<'BAR'
?>
BAR;
{$d}foo = b<<<'BAR'
?>
BAR;
if (0) <a />;
EOF;

$preprocess = xhp_preprocess_code($code);
if (empty($preprocess['new_code'])) {
  echo "No nowdocs!";
}
echo "pass";
--EXPECT--
pass
