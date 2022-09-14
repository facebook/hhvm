// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use ocamlrep::FromOcamlRep;

type OcamlValue = usize;

#[no_mangle]
unsafe extern "C" fn ocamlrep_marshal_output_value_to_string(
    v: OcamlValue,
    flags: OcamlValue,
) -> OcamlValue {
    ocamlrep_ocamlpool::catch_unwind(|| {
        let v = ocamlrep::Value::from_bits(v);
        let flags = ocamlrep_marshal::ExternFlags::from_ocaml(flags).unwrap();
        let mut cursor = std::io::Cursor::new(vec![]);
        ocamlrep_marshal::output_value(&mut cursor, v, flags).unwrap();
        ocamlrep_ocamlpool::to_ocaml(&cursor.into_inner())
    })
}

#[no_mangle]
unsafe extern "C" fn ocamlrep_marshal_input_value_from_string(
    str: OcamlValue,
    ofs: OcamlValue,
) -> OcamlValue {
    ocamlrep_ocamlpool::catch_unwind(|| {
        let offset = usize::from_ocaml(ofs).unwrap();
        let str = ocamlrep::bytes_from_ocamlrep(ocamlrep::Value::from_bits(str)).unwrap();
        let str = &str[offset..];
        let pool = ocamlrep_ocamlpool::Pool::new();
        ocamlrep_marshal::input_value(str, &pool).to_bits()
    })
}
