<?hh

function f(): void {
  darray[1 => 2];
  dict['a' => 'b', 'c' => 'd'];

  if (dict['k' => true]['k']) {
    darray[];
  } else {
    dict[];

  }

  try {
    1 == 2 ? darray[] : dict[];
  } catch (Exception $_) {
    dict[42 => 'a'];
  }
}
