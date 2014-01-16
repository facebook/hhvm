<?php
ini_set('session.save_handler', files);

ini_set('session.serialize_handler', php);

ini_set('session.use_cookies', 0);



echo "*** Test substituting argument 1 with unknown string values ***\n";



$heredoc = <<<EOT
hello world
EOT;

$variation_array = array(
  'string DQ' => "string",
  'string SQ' => 'string',
  'mixed case string' => "sTrInG",
  'heredoc' => $heredoc
  );


foreach ( $variation_array as $var ) {
  var_dump(get_cfg_var( $var  ) );
}
?>