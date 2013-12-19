<?php
ini_set('session.save_handler', files);

ini_set('session.serialize_handler', php);

ini_set('session.use_cookies', 0);



echo "*** Test substituting argument 1 with object values ***\n";



class classWithToString
{
        public function __toString() {
                return "session.use_cookies";
        }
}

class classWithoutToString
{
}

$variation_array = array(
  'instance of classWithToString' => new classWithToString(),
  'instance of classWithoutToString' => new classWithoutToString(),
  );


foreach ( $variation_array as $var ) {
  var_dump(get_cfg_var( $var  ) );
}
?>