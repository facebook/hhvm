<?hh

function bug( $flag ) :mixed{
  $tag = '';
  if ($flag) {
    $tag .= 'x';
  }
  $tag='33';
  if ( $flag ) ;
 else var_dump($tag);
}

<<__EntryPoint>>
function main_1745() :mixed{
bug(false);
}
