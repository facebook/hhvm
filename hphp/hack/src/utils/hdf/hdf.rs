// Copyright (c) Meta Platforms, Inc. and affiliates.
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

#[cxx::bridge(namespace = "HPHP")]
pub(crate) mod ffi {
    unsafe extern "C++" {
        include!("hphp/hack/src/utils/hdf/hdf-wrap.h");
        type Hdf;
        fn hdf_new() -> UniquePtr<Hdf>;
        fn hdf_new_child(parent: &Hdf, name: &CxxString) -> UniquePtr<Hdf>;
        fn hdf_first_child(parent: &Hdf) -> Result<UniquePtr<Hdf>>;
        fn hdf_next(hdf: &Hdf) -> Result<UniquePtr<Hdf>>;
        fn hdf_name(hdf: &Hdf) -> Result<String>;
        fn hdf_child_names(hdf: &Hdf) -> Result<Vec<String>>;

        fn append(self: Pin<&mut Hdf>, filename: &CxxString) -> Result<()>;
        fn fromString(self: Pin<&mut Hdf>, input: &CxxString) -> Result<()>;
        fn remove(self: &Hdf, name: &CxxString) -> Result<()>;
        fn copy(self: Pin<&mut Hdf>, dest: &Hdf) -> Result<()>;

        fn configGetBool(self: &Hdf, or_default: bool) -> Result<bool>;
        fn configGetUInt32(self: &Hdf, or_default: u32) -> Result<u32>;
        fn configGetInt64(self: &Hdf, or_default: i64) -> Result<i64>;
        unsafe fn configGet(self: &Hdf, or_default: *const c_char) -> Result<*const c_char>;
        fn exists(self: &Hdf) -> Result<bool>;
        fn toString(self: &Hdf) -> Result<*const c_char>;

        // Only used in tests
        #[allow(dead_code)]
        fn isEmpty(self: &Hdf) -> bool;
    }
}

#[cfg(test)]
mod test {
    use std::ffi::CStr;

    use cxx::let_cxx_string;
    use cxx::UniquePtr;

    use super::*;

    fn abc() -> UniquePtr<ffi::Hdf> {
        let mut hdf = ffi::hdf_new();
        assert!(hdf.isEmpty());
        let_cxx_string!(opt = "a.b.c=true");
        hdf.pin_mut().fromString(&opt).unwrap();
        hdf
    }

    #[test]
    fn test1() {
        let hdf = abc();
        assert_eq!(ffi::hdf_name(&hdf).unwrap(), "");
        let_cxx_string!(abc = "a.b.c");
        let abc = ffi::hdf_new_child(&hdf, &abc);
        assert_eq!(ffi::hdf_name(&abc).unwrap(), "c");
        assert!(abc.configGetBool(false).unwrap());
        assert!(abc.configGetBool(false).unwrap());
        let_cxx_string!(a = "a");
        let a = ffi::hdf_new_child(&hdf, &a);
        assert!(!a.isEmpty());
        let_cxx_string!(b = "b");
        let b = ffi::hdf_new_child(&a, &b);
        assert!(!b.isEmpty());
        let_cxx_string!(c = "c");
        let c = ffi::hdf_new_child(&b, &c);
        assert_eq!(ffi::hdf_name(&c).unwrap(), "c");
        assert!(!c.isEmpty());
        assert!(!b.configGetBool(false).unwrap());
        assert!(b.configGetBool(true).unwrap());
        assert!(c.configGetBool(false).unwrap());
        assert_eq!(ffi::hdf_child_names(&hdf).unwrap(), vec!["a"]);
    }

    #[test]
    fn test2() {
        let hdf = abc();
        let cstr = hdf.toString().unwrap();
        assert_ne!(cstr, std::ptr::null());
        let cstr = unsafe { CStr::from_ptr(cstr) };
        assert_eq!(cstr.to_str().unwrap(), "a {\n  b {\n    c = true\n  }\n}\n");
    }
}
