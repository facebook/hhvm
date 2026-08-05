//// prototypes/alpha/a.php
<?hh
// belongs to synthesized package prototypes.alpha
const int ALPHA_C = 1;

//// standalone/s.php
<?hh
// standalone does not include any prototypes member: cross-package violation
const int STANDALONE_C = ALPHA_C;
