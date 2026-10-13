//// base-a.php
<?hh
new module a {}

//// base-K.php
<?hh
/**
 * Removing the module declaration must recheck this constant-only file.
 * The expected fanout includes K so that its unbound module is diagnosed.
 */
module a;
const int K = 1;

//// changed-a.php
<?hh
