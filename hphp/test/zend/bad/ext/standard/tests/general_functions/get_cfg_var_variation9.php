<?php


echo "*** Test substituting argument with array of valid parameters ***\n";



$heredoc = <<<EOT
hello world
EOT;

$variation_array = array(
  'session.use_cookies',
  'session.serialize_handler', 
  'session.save_handler'
  );


foreach ( $variation_array as $var ) {
  var_dump(get_cfg_var( $var  ) );
}
?>
