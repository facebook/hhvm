<?hh

// NOTE: RepoAuth mode doesn't properly handle the first constant test,
//       a separate .expectf-repo is included until that is resolved.
// echo great[1];
// echo "\n";
const great = "my";
<<__EntryPoint>> function main(): void {
echo great[1];
}
