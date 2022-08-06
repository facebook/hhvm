// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use arena_deserializer::serde::Deserialize;
use typing_defs_rust::typing_defs_core::Ty;

pub fn into_types<'a>(
    arena: &'a bumpalo::Bump,
    chunks: impl Iterator<Item = Vec<u8>>,
) -> impl Iterator<Item = Ty<'a>> {
    let op = bincode::config::Options::with_native_endian(bincode::options());
    chunks.map(move |bs| {
        let mut de = bincode::de::Deserializer::from_slice(&bs, op);
        to_type(arena, &bs, &mut de)
    })
}

fn to_type<'a, 'de>(
    arena: &'a bumpalo::Bump,
    bs: &[u8],
    de: impl serde::Deserializer<'de>,
) -> Ty<'a> {
    let de = arena_deserializer::ArenaDeserializer::new(arena, de);
    let ty = Ty::deserialize(de);
    ty.unwrap_or_else(|_| panic!("cannot deserialize Ty from: {:?}", bs))
}

#[cfg(test)]
mod tests {
    use serde_json::json;
    use serde_json::value::Value as Json;

    use super::*;

    fn roundtrip(js: &[Json]) -> Vec<Json> {
        trait JsonRoundtrip: AsMut<Vec<Json>> {
            fn on_type(&mut self, ty: &Ty<'_>) {
                let j = serde_json::to_value(ty).unwrap_or_else(|_| json!(""));
                self.as_mut().push(j);
            }
        }

        impl JsonRoundtrip for Vec<Json> {}

        let mut act: Vec<Json> = Vec::new();
        let arena = bumpalo::Bump::new();
        let chunks = js
            .iter()
            .map(|j| serde_json::to_string(j).unwrap().into_bytes());
        for chunk in chunks {
            let mut de = serde_json::Deserializer::from_slice(&chunk);
            let ty = to_type(&arena, &chunk, &mut de);
            act.on_type(&ty)
        }
        act
    }

    #[test]
    fn de_ser_single_tprim() {
        let js = vec![json!([
          {"Rwitness": {
            "Tiny": {
              "file": "tmp|dir/file.php",
              "span": 123,
            },
          }},
          {"Tprim"
          : "Tnull"},
        ])];
        assert_eq!(roundtrip(&js), js);
    }

    #[test]
    fn de_ser_single_function_hint() {
        let (span1, span2, span3) = (347446144140693i64, 347446144141315i64, 347446144141700i64);
        let js = vec![
            // function higher_order((function(int): void) $_): void {}
            json!([
              {"Rhint":{"Tiny":{"file":"|f2.php","span": span1}}},
              {"Tfun": {
                "flags": 0,
                "ifc_decl": {"FDPolicied":"PUBLIC"},
                "implicit_params": {
                  "capability": {
                    "CapDefaults": {"Tiny":{"file":"|f2.php","span": span1}}
                  }
                },
                "params": [{
                  "flags":0,
                  "name":null,
                  "pos":{"Tiny":{"file":"|f2.php","span": span2}},
                  "type_": {
                    "enforced":"Unenforced",
                    "type_":[{"Rhint":{"Tiny":{"file":"|f2.php","span": span2}}},{"Tprim":"Tint"}]
                  }
                }],
                "ret": {
                  "enforced":"Unenforced",
                  "type_": [{"Rhint":{"Tiny":{"file":"|f2.php","span": span3}}},{"Tprim":"Tvoid"}]
                },
                "tparams": [],
                "where_constraints":[]}
              },
            ]),
        ];
        assert_eq!(roundtrip(&js), js);
    }

    #[test]
    fn de_ser_const_vec_arraykey_eq_vec_string_and_int() {
        // JSONs of types in: const vec<arraykey> AK = vec["4", 7];
        let js = vec![
            json!([{"Rwitness":{"Tiny":{"file":"|f.php","span":26388480395075i64}}},{"Tprim":"Tstring"}]),
            json!([{"Rwitness":{"Tiny":{"file":"|f.php","span":26388480395393i64}}},{"Tprim":"Tint"}]),
        ];
        assert_eq!(roundtrip(&js), js);
    }
}
