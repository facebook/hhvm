<?hh

namespace HH {
  // This exists as a stub: right now the user needs to provide their own HHI
  function is_any_array(mixed $_): bool {
    invariant_violation('!!!');
  }
}

namespace {

function main(mixed $x): void {
  if (HH\is_any_array($x)) {
    takes_container($x);
    takes_vec($x);
  } else {
    takes_container($x);
  }
}

function takes_container(Container<mixed> $_): void {}
function takes_vec(vec<mixed> $_): void {}

}
