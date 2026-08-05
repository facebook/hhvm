//// standalone/s.php
<?hh
// belongs to package standalone
const int STANDALONE_C = 1;

//// prototypes/alpha/a.php
<?hh
// prototypes.alpha only soft-includes standalone: access is reported as a
// soft-include cross-package access
const int ALPHA_C = STANDALONE_C;
