<?hh

function f(): darray<string, mixed> {
  return darray['a' => 42];
}

function g(): darray<string, mixed> {
  return dict['a' => 42];
}

function h(): dict<string, mixed> {
  return darray['a' => 42];
}
