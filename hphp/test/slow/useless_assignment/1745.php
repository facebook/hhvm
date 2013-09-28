<?php

function bug( $flag ) {
  $tag = '';
  if ($flag) {
    $tag .= 'x';
  }
  $tag='33';
  if ( $flag ) ;
 else var_dump($tag);
}
bug(false);
