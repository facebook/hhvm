//// newtype.php
<?hh
newtype FBID = int;

//// usage.php
<?hh

class C {
  /* HH_FIXME[4338] */ const FBID mark = 4;
}
