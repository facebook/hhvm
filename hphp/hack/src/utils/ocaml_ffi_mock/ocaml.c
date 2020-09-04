/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 */

void ocamlpool_enter(void) {}
void ocamlpool_leave(void) {}
void ocamlpool_cursor(void) {}
void ocamlpool_limit(void) {}
void ocamlpool_reserve_block(void) {}
void ocamlpool_color(void) {}
void ocamlpool_bound(void) {}
void caml_alloc(void) {}
void caml_alloc_small(void) {}
void caml_alloc_tuple(void) {}
void caml_array_length(void) {}
void caml_initialize(void) {}
void caml_is_double_array(void) {}
void caml_local_roots(void) {}
void caml_modify(void) {}
void ocamlpool_reserve_string(void) {}
void caml_alloc_string(void) {}
void caml_copy_double(void) {}
void caml_copy_int32(void) {}
void caml_copy_int64(void) {}
void caml_copy_nativeint(void) {}
void caml_enter_blocking_section(void) {}
void caml_failwith_value(void) {}
void caml_failwith(void) {}
void caml_invalid_argument_value(void) {}
void caml_leave_blocking_section(void) {}
void caml_raise(void) {}
void caml_raise_constant(void) {}
void caml_register_global_root(void) {}
void caml_remove_global_root(void) {}
void caml_string_length(void) {}
void caml_array_bound_error(void) {}
void caml_raise_end_of_file(void) {}
void caml_raise_not_found(void) {}
void caml_raise_out_of_memory(void) {}
void caml_raise_stack_overflow(void) {}
void caml_raise_sys_blocked_io(void) {}
void caml_raise_sys_error(void) {}
void caml_raise_with_arg(void) {}
void caml_raise_zero_divide(void) {}
void caml_named_value(void) {}
void caml_callbackN(void) {}
void caml_raise_with_string(void) {}

int ocamlpool_generation = 0;
