<?hh
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *
 */

async function fetch_db_results(): Awaitable<string> {
  return "Results!!!";
}

async function return_db_results(): Awaitable<string> {
  $a = await fetch_db_results();
  return "the results are".$a;
}
