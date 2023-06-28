<?hh

final class :my-xhp {}
final class :other-xhp {}

type TXHP = :my-xhp;

function is_xhp(mixed $x): void {
  if ($x is TXHP) {
    echo "xhp\n";
  } else {
    echo "not xhp\n";
  }
}


<<__EntryPoint>>
function main_is_expression_ta_xhp() :mixed{
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
