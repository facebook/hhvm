// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use arena_deserializer::serde::Deserialize;
use typing_defs_rust::typing_defs_core::Ty;

pub fn into_types<'a, I: AsRef<str>>(
    arena: &'a bumpalo::Bump,
    jsons: impl Iterator<Item = I>,
) -> impl Iterator<Item = Ty<'a>> {
    jsons.map(|json| {
        let json = json.as_ref();
        let mut de = serde_json::Deserializer::from_str(json);
        let de = arena_deserializer::ArenaDeserializer::new(arena, &mut de);
        let ty: serde_json::Result<Ty<'_>> = Ty::deserialize(de);
        ty.unwrap_or_else(|_| panic!("cannot deserialize Ty from JSON: {}", json))
    })
}

#[cfg(test)]
mod tests {
    use super::*;

    use serde_json::{json, value::Value as Json};

    fn roundtrip(js: &[Json]) -> Vec<Json> {
        trait JsonRoundtrip: AsMut<Vec<Json>> {
            fn on_type(&mut self, ty: &Ty<'_>) {
                let j = serde_json::to_value(ty).unwrap_or(json!(""));
                self.as_mut().push(j);
            }
        }

        impl JsonRoundtrip for Vec<Json> {}

        let mut act: Vec<Json> = Vec::new();
        let arena = bumpalo::Bump::new();
        let j_strs = js.into_iter().map(|j| serde_json::to_string(j).unwrap());
        for ty in into_types(&arena, j_strs) {
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
                "arity": "Fstandard",
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
            json!([{"Rwitness":{"Tiny":{"file":"|f.php","span":26388480394827i64}}},{"Tclass":[[{"Tiny":{"file":"|f.php","span":26388480394827i64}},"\\HH\\vec"],"Nonexact",[[{"Rhint":{"Tiny":{"file":"|f.php","span":26388480393864i64}}},{"Tprim":"Tarraykey"}]]]}]),
            json!([{"Rwitness":{"Tiny":{"file":"|f.php","span":26388480395075i64}}},{"Tprim":"Tstring"}]),
            json!([{"Rwitness":{"Tiny":{"file":"|f.php","span":26388480395393i64}}},{"Tprim":"Tint"}]),
        ];
        assert_eq!(roundtrip(&js), js);
    }
}
