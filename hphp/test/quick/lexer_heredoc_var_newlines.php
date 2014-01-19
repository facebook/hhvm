<?php
  $info = "hello";
echo <<<SCRIPT
<?php
$info
throw new Exception(<<<TXT
$info

Fix the above and then run `arc build`
TXT
);
SCRIPT;
