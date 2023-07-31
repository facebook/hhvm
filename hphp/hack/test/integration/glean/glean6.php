//// main.php
<?hh
// This test runs against the "www.hack.light" glean database, which stores predicates from the hack.6 schema fbcode/glean/schema/source/hack.angle
// RUN: %hh_single_complete --auto-complete --glean-reponame www.hack.light --auto-namespace-map '{"Vec": "FlibSL\\Vec"}' %s | FileCheck %s

//// glean1_vec.php
<?hh
function glean1_vec(): void { VecAUTO332
// CHECK: //// glean1_vec.php
// **not_yet_working**: INSERT Vec
// **not_yet_working-next**: namespace

//// glean1_bvec.php
<?hh
function glean1_bvec(): void { \VecAUTO332
// CHECK: //// glean1_bvec.php
// **not_yet_working**: INSERT Vec
// **not_yet_working-next**: namespace

//// glean1_vec_chunk.php
<?hh
function glean1_vec_chunk(): void { Vec\chAUTO332
// CHECK: //// glean1_vec_chunk.php
// CHECK: INSERT chunk
// CHECK-NEXT: function

//// glean1_bvec_chunk.php
<?hh
function glean1_bvec_chunk(): void { \Vec\chAUTO332
// CHECK: //// glean1_bvec_chunk.php
// **not_yet_working**: INSERT chunk
// **not_yet_working-next**: function

//// glean1_chessbot.php
<?hh
function glean1_chessbot(): void { ChessBoAUTO332
// CHECK: //// glean1_chessbot.php
// CHECK: INSERT ChessBot
// CHECK-NEXT: class

//// glean1_bchessbot.php
<?hh
function glean1_bchessbot(): void { \ChessBoAUTO332
// CHECK: //// glean1_bchessbot.php
// CHECK: INSERT ChessBot
// CHECK-NEXT: class

//// glean1_utf8.php
<?hh
function glean1_utf8(): void { utf8_strleAUTO332
// CHECK: //// glean1_utf8.php
// CHECK: INSERT utf8_strlen
// CHECK-NEXT: function

//// glean1_butf8.php
<?hh
function glean1_butf8(): void { \utf8_strleAUTO332
// CHECK: //// glean1_butf8.php
// CHECK: INSERT utf8_strlen
// CHECK-NEXT: function
