//// prototypes/loose.php
<?hh
// Directly under the family path: reported by check_directly_under_family.
const int LOOSE_C = 1;

//// prototypes/1bad/b.php
<?hh
// Under a directory that cannot name a member: reported by
// check_invalid_member_dir.
const int BAD_C = 1;

//// prototypes/ok/o.php
<?hh
// Correctly placed, so neither check fires for it.
const int OK_C = 1;
