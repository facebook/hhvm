<?hh

function bug( $flag ) {
  $tag = '';
  if ($flag) {
    $tag .= 'x';
  }
  $tag='33';
  if ( $flag ) ;
 else var_dump($tag);
}

<<__EntryPoint>>
function main_1745() {
bug(false);
}
