<?hh

newtype UserID = int;

function make_user_id(UserID $id): UserID { return $id; }
<<__EntryPoint>> function main(): void { echo "Done.\n"; }
