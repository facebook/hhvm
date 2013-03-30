<?php
$code = <<<'EOF'
<?php
  $x = <<<'EOT'
some string    
EOT
  $y = 2;
?>
EOF;
highlight_string($code);
?>