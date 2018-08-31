<?php

<<__EntryPoint>>
function main_issue_3160() {
$str = <<<'EOL'
  <?php
    echo "$var";
    echo "text $var text";
    echo "text";
  ?>
EOL;

highlight_string($str);
}
