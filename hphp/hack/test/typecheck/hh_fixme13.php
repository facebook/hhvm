<?hh // partial

function f(string $s): void {}

function g(): void {
  $xhp =
    <p>
      /* HH_FIXME[4110] This is actually part of the XHP and will be rendered to
         the page, and so is not safe and shouldn't be accepted as a FIXME! */
      {f(1)}
    </p>;
}

class :p {}
