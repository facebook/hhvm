////file1.php
<?hh

interface I<T> {}
interface I2 extends I<int> {}
/* HH_FIXME[4101] */
function i(): I {
  // UNSAFE
}

////file2.php
<?hh // strict

function test(): I2 {
  /* HH_FIXME[4029] */
  return i();
}
