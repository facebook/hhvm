<?hh

final class :my-xhp {}
final class :other-xhp {}

function is_xhp(mixed $x): void {
  try {
    var_dump($x as :my-xhp);
  } catch (TypeAssertionException $e) {
    echo $e->getMessage()."\n";
  }
}


<<__EntryPoint>>
function main_xhp() :mixed{
is_xhp(1);
is_xhp(1.5);
is_xhp('foo');
is_xhp(false);
is_xhp(fopen(__FILE__, 'r'));
is_xhp(new stdClass());
is_xhp(null);
is_xhp(vec[]);
is_xhp(dict[]);
is_xhp(keyset[]);
is_xhp(<my-xhp />);
is_xhp(<other-xhp />);
}
