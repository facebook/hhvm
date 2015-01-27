<?hh

function f(string $s): void {}

function g(): void {
  $xhp =
    <p
      /* HH_FIXME[4110] in XHP attrs */
      class={f(1)}>
      para
    </p>;
}
