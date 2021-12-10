use naming_special_names_rust as naming_special_names;

#[allow(clippy::needless_lifetimes)]
#[no_mangle]
pub unsafe extern "C" fn no_call_compile_only_USED_TYPES_naming_special_names(
    _: naming_special_names::coeffects::Ctx,
) {
    unimplemented!()
}
