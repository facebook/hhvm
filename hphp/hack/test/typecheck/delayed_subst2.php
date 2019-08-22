////file1.php
<?hh // partial

interface I<T> {}
interface I2 extends I<int> {}
/* HH_FIXME[4101] */
/* HH_FIXME[4336] */
function i(): I {
}

////file2.php
<?hh // strict

function test(): I2 {
  /* HH_FIXME[4029] */
  return i();
}
