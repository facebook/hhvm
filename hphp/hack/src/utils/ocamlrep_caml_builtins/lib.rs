use ocamlrep::*;
use ocamlrep_custom::CustomOperations;

pub struct Int64(pub i64);

extern "C" {
    static mut caml_int64_ops: CustomOperations;
}

impl ToOcamlRep for Int64 {
    fn to_ocamlrep<'a, A: Allocator>(&self, alloc: &'a A) -> OpaqueValue<'a> {
        let mut block = alloc.block_with_size_and_tag(2, CUSTOM_TAG);
        alloc.set_field(&mut block, 0, unsafe {
            OpaqueValue::from_bits((&caml_int64_ops) as *const CustomOperations as usize)
        });
        alloc.set_field(&mut block, 1, unsafe {
            OpaqueValue::from_bits(self.0 as usize)
        });
        block.build()
    }
}

impl FromOcamlRep for Int64 {
    fn from_ocamlrep(value: Value<'_>) -> Result<Self, FromError> {
        let block = from::expect_block_with_size_and_tag(value, 2, CUSTOM_TAG)?;
        Ok(Self(block[1].to_bits() as i64))
    }
}
