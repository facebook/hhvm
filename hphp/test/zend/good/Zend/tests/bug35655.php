<?php
$code = '
<?php
  $x = <<<EOT
some string    
EOT
  $y = 2;
?>';
highlight_string($code);
?>