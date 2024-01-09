// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

/// Emit the opcodes enum. Given input that looks like this:
///
/// ```
/// #[macros::emit_opcodes]
/// enum MyCrazyOpcodes<'lifetime> {}
/// ```
///
/// The result will look something like this:
///
/// ```
/// enum MyCrazyOpcodes<'lifetime> {
///     Jmp(Label),
///     Nop,
///     PopL(Local<'lifetime>),
/// }
/// ```
///
/// If the 'Targets' derive is used then the Targets trait will be implemented
/// as well.
///
/// See emit_opcodes::tests::test_basic() for a more detailed example output.
#[proc_macro_attribute]
pub fn emit_opcodes(
    _attrs: proc_macro::TokenStream,
    input: proc_macro::TokenStream,
) -> proc_macro::TokenStream {
    match emit_opcodes::emit_opcodes(input.into(), hhbc_gen::opcode_data()) {
        Ok(res) => res.into(),
        Err(err) => err.into_compile_error().into(),
    }
}

#[proc_macro_derive(Targets)]
pub fn emit_targets(input: proc_macro::TokenStream) -> proc_macro::TokenStream {
    match emit_opcodes::emit_impl_targets(input.into(), hhbc_gen::opcode_data()) {
        Ok(res) => res.into(),
        Err(err) => err.into_compile_error().into(),
    }
}

#[proc_macro]
pub fn define_instr_seq_helpers(input: proc_macro::TokenStream) -> proc_macro::TokenStream {
    match emit_opcodes::define_instr_seq_helpers(input.into(), hhbc_gen::opcode_data()) {
        Ok(res) => res.into(),
        Err(err) => err.into_compile_error().into(),
    }
}
