//// prototypes/1bad/no_defs.php
<?hh
// `1bad` is not a valid identifier, and this file declares nothing at all.
// The invalid-member-dir check is registered as a whole-file check, so it still
// fires; a per-definition check would never run for this file.
