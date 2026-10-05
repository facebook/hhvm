//// key.php
<?hh

class Key {
  const FIELD = 'field';
}

//// child.php
<?hh

class ChildKey extends Key {}

//// use.php
<?hh

type S = shape(ChildKey::FIELD => int);

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
