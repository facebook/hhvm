<?hh

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
