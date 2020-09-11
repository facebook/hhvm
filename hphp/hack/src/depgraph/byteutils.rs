use std::convert::TryInto;
use std::slice::SliceIndex;

#[inline(always)]
pub fn subslice<'a, I, T>(
    bytes: &'a [T],
    index: I,
    descr: &str,
) -> Result<&'a <I as SliceIndex<[T]>>::Output, String>
where
    I: SliceIndex<[T]>,
{
    bytes
        .get(index)
        .ok_or_else(|| format!("not enough bytes while reading {}", descr))
}

/// Align a byte array to an u32-array.
///
/// Returns `None` if the byte array was not properly aligned.
///
/// Panics if `slice::align_to` behavior is unexpected.
#[inline(always)]
pub fn as_u32_slice(bytes: &[u8]) -> Option<&[u32]> {
    // Safety: Safe because:
    //
    //  1. The u32 has no invalid states
    //  2. The return type is well-defined
    //  3. We don't transmute to a mutable type
    //  4. We don't produce unbounded lifetimes
    //  5. We explicitly check the behavior of `align_to`
    let (prefix, slice, suffix) = unsafe { bytes.align_to::<u32>() };
    if !prefix.is_empty() {
        return None;
    }
    if suffix.len() >= std::mem::size_of::<u32>() {
        panic!("suffix too long");
    }
    Some(slice)
}

/// Align a byte array to an u64-array.
///
/// Returns `None` if the byte array was not properly aligned.
///
/// Panics if `slice::align_to` behavior is unexpected.
#[inline(always)]
pub fn as_u64_slice(bytes: &[u8]) -> Option<&[u64]> {
    // Safety: Safe because:
    //
    //  1. The u32 has no invalid states
    //  2. The return type is well-defined
    //  3. We don't transmute to a mutable type
    //  4. We don't produce unbounded lifetimes
    //  5. We explicitly check the behavior of `align_to`
    let (prefix, slice, suffix) = unsafe { bytes.align_to::<u64>() };
    if !prefix.is_empty() {
        return None;
    }
    if suffix.len() >= std::mem::size_of::<u64>() {
        panic!("suffix too long");
    }
    Some(slice)
}

#[inline(always)]
pub fn read_u32_ne(bytes: &[u8]) -> u32 {
    u32::from_ne_bytes(bytes[0..4].try_into().unwrap())
}

#[inline(always)]
pub fn read_u64_ne(bytes: &[u8]) -> u64 {
    u64::from_ne_bytes(bytes[0..8].try_into().unwrap())
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_read_u32_ne() {
        let v: u32 = 0x45831546;
        let b: [u8; 4] = v.to_ne_bytes();
        assert_eq!(v, read_u32_ne(&b));
    }

    #[test]
    fn test_read_u64_ne() {
        let v: u64 = 0x45831546;
        let b: [u8; 8] = v.to_ne_bytes();
        assert_eq!(v, read_u64_ne(&b));
    }
}
