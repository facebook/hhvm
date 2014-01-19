<?php


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