////file1.php
<?hh
type UNSAFE_TYPE_HH_FIXME_<T> = T;

////file2.php
<?hh

/* HH_FIXME[4101] Please add the expected type parameters */
type UNSAFE_TYPE_HH_FIXME = UNSAFE_TYPE_HH_FIXME_;

////file3.php
<?hh // partial

function f(): UNSAFE_TYPE_HH_FIXME {
  return "foo";
}

////file4.php

abstract final class HH_FIXME {
  const type MISSING_TYPE = UNSAFE_TYPE_HH_FIXME;
}

////file5.php
<?hh // partial

function g(): HH_FIXME::MISSING_TYPE {
  return 0;
}
