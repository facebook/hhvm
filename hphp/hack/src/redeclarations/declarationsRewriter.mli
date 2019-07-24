(**
 * Copyright (c) 2019, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *)

(**
 * Start running in a loop that will wait for code blocks at standard input
 * and will output the same code blocks, but modified, such that redeclared
 * functions will be supported.
 * Each code block should end with the line "_CODE_BLOCK_END_". This will
 * trigger the processing of the input.
 * Break the loop when EOF is encountered at standard input.
 *)
val start : unit -> unit
