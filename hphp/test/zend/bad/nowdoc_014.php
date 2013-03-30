<?php
$code = <<<'EOF'
<?php
  $x = <<<'EOT'
EOT
  $y = 2;
?>
EOF;
highlight_string($code);
?>