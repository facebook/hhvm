<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function reduce<Tv, Ta>(
  Traversable<Tv> $traversable,
  (function(Ta, Tv): Ta) $accumulator,
  Ta $initial,
): Ta {
  return $initial;
}
function testit(varray<shape('page_id' => string, 'page_name' => string)> $v):void {
  // So: Tv appears in a covariant position in a lambda parameter type
  // Ta appears in a covariant position in the result
  // And we have varray<shape(...) <: Traversable<Tv>
  // So shape(...) <: Tv
  // Also vec<nothing> <: Ta
  // But
  $pages_document = reduce<_,_>($v, ($pages_document, $res) ==> {
      //hh_show_env();
      $pages_document[] = Map {
        "uid" => (string)$res['page_id'],
        "text" => $res['page_name'],
      };
      //hh_show_env();
      return $pages_document;
    }, vec[]);
}
