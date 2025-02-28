(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** * Log whether `new`s and static method calls are abstract-safe.
 * For example, `new $cls()` is only abstract-safe if we know that `$cls` refers to a concrete class.

 * Invariant: The calculation is only correct if there are no type errors.
 * This is because Safe_abstract_internal.calc_warnings avoids redundancy with checks elsewhere in the type checker.
 * So (for example) `new C()` is always considered safe when `C` is an id rather than a variable or `static`.
 *
 * Throws if `safe_abstract=true` is not set in the config
 *
 * Configuration: in .hh_config `log_levels={"safe_abstract": THE_LEVEL}`
 * - when THE_LEVEL=1 log summaries for each class and top-level function
 * - when THE_LEVEL=2 log for each `new` and static call expression

 *)
val create_handler : Provider_context.t -> Tast_visitor.handler_base
