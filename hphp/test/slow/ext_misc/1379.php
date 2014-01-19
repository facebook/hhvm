<?php

var_dump(token_name(396));
$str = "<?php

\$x=<foo:bar/>;
\$y=<foo:bar>woo!</foo:bar>;
\n";
$arr = token_get_all($str);
foreach ($arr as $t) {
  if ($t[0] == 396) {
    var_dump($t[1]);
  }
}
