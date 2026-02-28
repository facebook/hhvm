//// base-foo.php
<?hh
const int C = 3;
//// base-a.php
<?hh
function get_c(): int { return C; }
//// changed-foo.php
<?hh
const string C = 3;
//// changed-a.php
<?hh
function get_c(): int { return C; }
