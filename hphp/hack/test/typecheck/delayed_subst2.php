////file1.php
<?hh

interface I2 { }
interface I<T> extends I2 {}

/* HH_FIXME[4101] */
function i(): I {
  throw new Exception();
}

////file2.php
<?hh// strict

function test(): I2 {
  /* HH_FIXME[4029] */
  return i();
}
