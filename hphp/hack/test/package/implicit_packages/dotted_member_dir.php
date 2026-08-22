//// prototypes/proto.v1/a.php
<?hh
// `proto.v1` contains a dot, so it is not a valid identifier and cannot name an
// implicit package member. Supporting it would also produce the member name
// `prototypes.proto.v1`, which does not round-trip through the family/member
// split. The file belongs to no package and its placement is an error.
const int V1_C = 1;
