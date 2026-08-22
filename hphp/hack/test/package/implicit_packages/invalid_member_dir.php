//// prototypes/1bad/a.php
<?hh
// Leading digit: not a valid identifier.
const int BAD_DIGIT_C = 1;

//// prototypes/:xhp/b.php
<?hh
// XHP-style name: `:` is not a name character.
const int BAD_XHP_C = 1;

//// prototypes/foo/ok.php
<?hh
// `foo` is a valid identifier, so this file belongs to `prototypes.foo` and its
// placement is fine.
const int FOO_C = 1;
