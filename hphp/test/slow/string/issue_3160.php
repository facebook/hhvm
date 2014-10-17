<?php
$str = <<<'EOL'
  <?php
    echo "$var";
    echo "text $var text";
    echo "text";
  ?>
EOL;

highlight_string($str);
