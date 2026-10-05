//// key.php
<?hh

class Key {
  const FIELD = 'field';
}

//// use.php
<?hh

type S = shape(Key::FIELD => int);

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
