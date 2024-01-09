// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

// ----------------------------------------------------------------------------

/// Emit a formatter to print opcodes.
///
/// Given input that looks like this:
/// ```
/// #[derive(macros::OpcodeDisplay)]
/// struct PrintMe(Opcodes);
/// ```
///
/// This will add an impl for Display that looks something like this:
///
/// ```
/// impl<T> OpcodeDisplay for PrintMe<T> {
///     fn print_opcode(&self, w: &mut Self::Write) -> Result<(), Self::Error> {
///         match self.0 {
///             PrintMe::Jmp(target1) => {
///                 write!(w, "Jmp ")?;
///                 print_label(w, target1)?;
///             }
///         }
///     }
/// }
/// ```
///
/// See print_opcode_derive::tests::test_basic() for a more detailed example
/// output.
#[proc_macro_derive(PrintOpcode, attributes(print_opcode))]
pub fn print_opcode_macro(input: proc_macro::TokenStream) -> proc_macro::TokenStream {
    match print_opcode_impl::build_print_opcode(input.into(), hhbc_gen::opcode_data()) {
        Ok(res) => res.into(),
        Err(err) => err.into_compile_error().into(),
    }
}
