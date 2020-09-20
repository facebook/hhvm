<?hh

function main(mixed $x): void {
  if (HH\is_vec_or_varray($x)) {
    // these work
    takes_container($x);
    $x[] = 5;
    $x[3] = 3;

    // this raise type errors
    $x['abc'] = 3;
    takes_dict($x);
    takes_vec($x);
    takes_darray($x);
    takes_varray($x);
    takes_varray_or_darray($x);
  } else {
    $x[] = 5;
  }
}

function takes_container(Container<mixed> $_): void {}
function takes_vec(vec<mixed> $_): void {}
function takes_dict(dict<arraykey, mixed> $_): void {}
function takes_varray(varray<mixed> $_): void {}
function takes_darray(darray<arraykey, mixed> $_): void {}
function takes_varray_or_darray(varray_or_darray<arraykey, mixed> $_): void {}
