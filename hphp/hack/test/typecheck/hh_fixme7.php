//// newtype.php
<?hh // strict
newtype FBID = int;

//// usage.php
<?hh // strict

class C {
  /* HH_FIXME[4110] */ const FBID mark = 4;
}
