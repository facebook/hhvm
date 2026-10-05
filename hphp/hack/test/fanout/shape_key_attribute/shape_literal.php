//// key.php
<?hh

class Key {
  const FIELD = 'field';
}

//// use.php
<?hh

function make_shape(): void {
  $s = shape(Key::FIELD => 1);
}

//////////////////////

//// key.php
<?hh

class Key {
  <<__ShapeKey>>
  const FIELD = 'field';
}

//////////////////////

//// key.php
<?hh

class Key {
  const FIELD = 'field';
}

//////////////////////

//// key.php
<?hh

class Key {
  <<__ShapeKey>>
  const FIELD = 'field';
}
