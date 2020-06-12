<?hh

// varray now only supports unsetting elements at the end of the array.
// Like vecs, unset on varray also resets the internal iterator position.
function h4() {
  $x = varray[17, 34, 54, 68];
  end(inout $x);
  next(inout $x);
  $y = $x;
  unset($y[3]);
  var_dump(current($x));
  var_dump(current($y));

  $x = vec[17, 34, 54, 68];
  end(inout $x);
  next(inout $x);
  $y = $x;
  unset($y[3]);
  var_dump(current($x));
  var_dump(current($y));
}

<<__EntryPoint>>
function main_253() {
h4();
}
