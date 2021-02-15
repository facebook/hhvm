<?hh // partial

function f(string $s): string {
  return $s;
}

function g(): void {
  $xhp =
    <p
      /* HH_FIXME[4110] in XHP attrs */
      class={f(1)}>
      para
    </p>;
}

class :p extends XHPTest {
  attribute string class;
}
