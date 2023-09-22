// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.
use std::fs::File;
use std::io::BufReader;
use std::path::Path;
use std::str::FromStr;

use anyhow::Context;
use anyhow::Result;
use hash::HashMap;
use hash::HashSet;
use serde_json;

use crate::custom_error::CustomError;
use crate::custom_error::VersionedErrorMessage;
use crate::custom_error::VersionedPattError;
use crate::custom_error_config::CustomErrorConfig;
use crate::error_message::Elem;
use crate::error_message::ErrorMessage;
use crate::patt_binding_ty::PattBindingTy;
use crate::patt_error::Callback;
use crate::patt_error::PattError;
use crate::patt_error::Primary;
use crate::patt_error::ReasonsCallback;
use crate::patt_error::Secondary;
use crate::patt_locl_ty::Params;
use crate::patt_locl_ty::PattLoclTy;
use crate::patt_locl_ty::Prim;
use crate::patt_locl_ty::ShapeField;
use crate::patt_locl_ty::ShapeFields;
use crate::patt_locl_ty::ShapeLabel;
use crate::patt_name::Namespace;
use crate::patt_name::PattName;
use crate::patt_string::PattString;
use crate::validation_err::ValidationErr;

impl CustomErrorConfig {
    pub fn new(mut errors: Vec<CustomError>) -> Self {
        let invalid = errors
            .extract_if(|e| {
                let mut env = ValidationEnv::default();
                !e.validate(&mut env)
            })
            .collect();
        Self {
            valid: errors,
            invalid,
        }
    }

    pub fn from_path(path: &Path) -> Result<CustomErrorConfig> {
        if path.exists() {
            let file = File::open(path).with_context(|| path.display().to_string())?;
            let reader = BufReader::new(file);
            let errors = serde_json::from_reader(reader)?;
            Ok(Self::new(errors))
        } else {
            Ok(Self::default())
        }
    }
}

impl Default for CustomErrorConfig {
    fn default() -> Self {
        Self {
            valid: vec![],
            invalid: vec![],
        }
    }
}

impl FromStr for CustomErrorConfig {
    type Err = anyhow::Error;
    fn from_str(contents: &str) -> Result<Self, Self::Err> {
        let errors: Vec<CustomError> = serde_json::from_str(contents)?;
        Ok(Self::new(errors))
    }
}

// -- Trait for types that can be validated ------------------------------------

pub trait Validatable {
    fn validate(&mut self, env: &mut ValidationEnv) -> bool;
}

trait Invalidatable {
    fn invalidate(&mut self, errs: &[ValidationErr]);
}

// -- Validation environment ---------------------------------------------------

#[derive(Debug, Default, Clone)]
pub struct ValidationEnv(HashMap<String, PattBindingTy>);

impl ValidationEnv {
    fn add(&mut self, name: &str, ty: PattBindingTy) -> Option<ValidationErr> {
        if self.0.contains_key(name) {
            Some(ValidationErr::Shadowed(name.to_string()))
        } else {
            self.0.insert(name.to_string(), ty);
            None
        }
    }

    fn get(&self, name: &str) -> Option<&PattBindingTy> {
        self.0.get(name)
    }

    fn join(&mut self, other: &Self) -> (Vec<ValidationErr>, Vec<ValidationErr>) {
        let mut errs1 = vec![];
        let mut errs2 = vec![];
        let ks1: HashSet<_> = self.0.keys().cloned().collect();
        let ks2: HashSet<_> = other.0.keys().cloned().collect();
        ks1.union(&ks2)
            .for_each(|k| match (self.0.get(k), other.0.get(k)) {
                (Some(PattBindingTy::Name), Some(PattBindingTy::Name)) => (),
                (Some(PattBindingTy::Ty), Some(PattBindingTy::Ty)) => (),
                (Some(l), Some(r)) => errs2.push(ValidationErr::Mismatch(l.clone(), r.clone())),
                (Some(_), _) => errs2.push(ValidationErr::Unbound(k.to_string())),
                (_, Some(b)) => {
                    self.0.insert(k.to_string(), b.clone());
                    errs1.push(ValidationErr::Unbound(k.to_string()))
                }
                _ => panic!("Impossible"),
            });
        (errs1, errs2)
    }
}

// -- Individual errors --------------------------------------------------------

impl Validatable for VersionedErrorMessage {
    fn validate(&mut self, env: &mut ValidationEnv) -> bool {
        match self {
            Self::MessageV1(msg) => msg.validate(env),
        }
    }
}

impl Validatable for VersionedPattError {
    fn validate(&mut self, env: &mut ValidationEnv) -> bool {
        match self {
            Self::ErrorV1(msg) => msg.validate(env),
        }
    }
}

impl Validatable for CustomError {
    fn validate(&mut self, env: &mut ValidationEnv) -> bool {
        self.patt.validate(env) && self.error_message.validate(env)
    }
}

// -- Error patterns -----------------------------------------------------------

impl Validatable for PattError {
    fn validate(&mut self, env: &mut ValidationEnv) -> bool {
        match self {
            Self::Invalid { .. } => false,
            Self::Primary(prim) => prim.validate(env),
            Self::Apply { cb, err } => cb.validate(env) && err.validate(env),
            Self::ApplyReasons { rsns_cb, secondary } => {
                rsns_cb.validate(env) && secondary.validate(env)
            }
            Self::Or { fst, snd } => {
                let mut env_snd = env.clone();
                let fst_valid = fst.validate(env);
                let snd_valid = snd.validate(&mut env_snd);
                // Determine the errors in the left- and right-hand side patterns
                // and add bindings present in the right but not the left side
                // to the environment
                let (errs_fst, errs_snd) = env.join(&env_snd);
                // Wrap the left and right side in an invalid marker based on the
                // join errors
                fst.invalidate(&errs_fst);
                snd.invalidate(&errs_snd);
                fst_valid && snd_valid && errs_fst.is_empty() && errs_snd.is_empty()
            }
        }
    }
}

impl Invalidatable for PattError {
    fn invalidate(&mut self, errs_in: &[ValidationErr]) {
        if !errs_in.is_empty() {
            match self {
                Self::Invalid { errs, .. } => errs.extend(errs_in.to_vec()),
                _ => {
                    let patt = std::mem::replace(self, PattError::Primary(Primary::AnyPrim));
                    *self = PattError::Invalid {
                        errs: errs_in.to_vec(),
                        patt: Box::new(patt),
                    };
                }
            }
        }
    }
}

impl Validatable for Secondary {
    fn validate(&mut self, env: &mut ValidationEnv) -> bool {
        match self {
            Self::AnySnd => true,
            Self::OfError(err) => err.validate(env),
            Self::ViolatedConstraint {
                cstr,
                ty_sub,
                ty_sup,
            } => cstr.validate(env) && ty_sub.validate(env) && ty_sup.validate(env),
            Self::SubtypingError { sub, sup } => sub.validate(env) && sup.validate(env),
        }
    }
}

impl Validatable for Primary {
    fn validate(&mut self, _env: &mut ValidationEnv) -> bool {
        true
    }
}

impl Validatable for Callback {
    fn validate(&mut self, _env: &mut ValidationEnv) -> bool {
        true
    }
}

impl Validatable for ReasonsCallback {
    fn validate(&mut self, _env: &mut ValidationEnv) -> bool {
        true
    }
}

// -- Type patterns ------------------------------------------------------------

impl Validatable for PattLoclTy {
    fn validate(&mut self, env: &mut ValidationEnv) -> bool {
        match self {
            Self::Invalid(..) => false,
            Self::Dynamic | Self::Nonnull | Self::Any => true,
            Self::Apply { name, params } => name.validate(env) && params.validate(env),
            Self::Prim(prim) => prim.validate(env),
            Self::Shape(fields) => fields.validate(env),
            Self::Option(patt) => patt.validate(env),
            Self::Tuple(patts) => patts.validate(env),
            Self::As { lbl, patt } => {
                let errs = env.add(lbl, PattBindingTy::Ty).map_or(vec![], |e| vec![e]);
                let valid = patt.validate(env);
                self.invalidate(&errs);
                valid && errs.is_empty()
            }
            Self::Or { fst, snd } => {
                let mut env_snd = env.clone();
                let fst_valid = fst.validate(env);
                let snd_valid = snd.validate(&mut env_snd);
                // Determine the errors in the left- and right-hand side patterns
                // and add bindings present in the right but not the left side
                // to the environment
                let (errs_fst, errs_snd) = env.join(&env_snd);
                // Wrap the left and right side in an invalid marker based on the
                // join errors
                fst.invalidate(&errs_fst);
                snd.invalidate(&errs_snd);
                fst_valid && snd_valid && errs_fst.is_empty() && errs_snd.is_empty()
            }
        }
    }
}

impl Invalidatable for PattLoclTy {
    fn invalidate(&mut self, errs_in: &[ValidationErr]) {
        if !errs_in.is_empty() {
            match self {
                Self::Invalid(errs, _patt) => errs.extend(errs_in.to_vec()),
                _ => {
                    let patt = std::mem::replace(self, PattLoclTy::Any);
                    *self = PattLoclTy::Invalid(errs_in.to_vec(), Box::new(patt));
                }
            }
        }
    }
}

impl Validatable for Params {
    fn validate(&mut self, env: &mut ValidationEnv) -> bool {
        match self {
            Self::Nil | Self::Wildcard => true,
            Self::Exists(patt) => patt.validate(env),
            Self::Cons { hd, tl } => hd.validate(env) && tl.validate(env),
        }
    }
}

impl Validatable for Prim {
    fn validate(&mut self, _env: &mut ValidationEnv) -> bool {
        true
    }
}

impl Validatable for ShapeLabel {
    fn validate(&mut self, _env: &mut ValidationEnv) -> bool {
        true
    }
}

impl Validatable for ShapeField {
    fn validate(&mut self, env: &mut ValidationEnv) -> bool {
        self.lbl.validate(env) && self.patt.validate(env)
    }
}

impl Validatable for ShapeFields {
    fn validate(&mut self, env: &mut ValidationEnv) -> bool {
        match self {
            Self::Fld { fld, rest } => fld.validate(env) && rest.validate(env),
            Self::Open | Self::Closed => true,
        }
    }
}

// -- Name patterns ------------------------------------------------------------

impl Validatable for PattName {
    fn validate(&mut self, env: &mut ValidationEnv) -> bool {
        match self {
            Self::As { lbl, patt } => {
                let errs = env
                    .add(lbl, PattBindingTy::Name)
                    .map_or(vec![], |e| vec![e]);
                let valid = patt.validate(env);
                self.invalidate(&errs);
                valid && errs.is_empty()
            }
            Self::Name { namespace, name } => namespace.validate(env) && name.validate(env),
            Self::Invalid { .. } => false,
            Self::Wildcard => true,
        }
    }
}

impl Invalidatable for PattName {
    fn invalidate(&mut self, errs_in: &[ValidationErr]) {
        if !errs_in.is_empty() {
            match self {
                Self::Invalid { errs, .. } => errs.extend(errs_in.to_vec()),
                _ => {
                    let patt = std::mem::replace(self, PattName::Wildcard);
                    *self = PattName::Invalid {
                        errs: errs_in.to_vec(),
                        patt: Box::new(patt),
                    };
                }
            }
        }
    }
}

impl Validatable for Namespace {
    fn validate(&mut self, _env: &mut ValidationEnv) -> bool {
        true
    }
}

// -- String patterns ----------------------------------------------------------

impl Validatable for PattString {
    fn validate(&mut self, _env: &mut ValidationEnv) -> bool {
        true
    }
}

// -- Error messages -----------------------------------------------------------

impl Validatable for ErrorMessage {
    fn validate(&mut self, env: &mut ValidationEnv) -> bool {
        self.message.validate(env)
    }
}

impl Validatable for Elem {
    fn validate(&mut self, env: &mut ValidationEnv) -> bool {
        match self {
            Self::TyVar(name) => matches!(env.get(name), Some(PattBindingTy::Ty)),
            Self::NameVar(name) => matches!(env.get(name), Some(PattBindingTy::Name)),
            Self::Lit(_) => true,
        }
    }
}

// -- Trait helpers ------------------------------------------------------------

impl<T> Validatable for Box<T>
where
    T: Validatable,
{
    fn validate(&mut self, env: &mut ValidationEnv) -> bool {
        (**self).validate(env)
    }
}

impl<T> Validatable for Vec<T>
where
    T: Validatable,
{
    fn validate(&mut self, env: &mut ValidationEnv) -> bool {
        let mut valid = true;
        for x in self {
            valid &= x.validate(env);
        }
        valid
    }
}

// -- Tests --------------------------------------------------------------------
#[cfg(test)]
mod tests {

    use super::*;

    #[test]
    fn test_validate_ty_unbound_right() {
        let fst = Box::new(PattLoclTy::As {
            lbl: "x".to_string(),
            patt: Box::new(PattLoclTy::Dynamic),
        });
        let snd = Box::new(PattLoclTy::Nonnull);
        let mut patt = PattLoclTy::Or { fst, snd };
        let mut env = ValidationEnv::default();
        let valid = patt.validate(&mut env);
        assert!(!valid);
        assert!(matches!(
            patt,
            PattLoclTy::Or {
                snd: box PattLoclTy::Invalid(..),
                ..
            }
        ));
    }

    #[test]
    fn test_validate_ty_unbound_left() {
        let snd = Box::new(PattLoclTy::As {
            lbl: "x".to_string(),
            patt: Box::new(PattLoclTy::Dynamic),
        });
        let fst = Box::new(PattLoclTy::Nonnull);
        let mut patt = PattLoclTy::Or { fst, snd };
        let mut env = ValidationEnv::default();
        let valid = patt.validate(&mut env);
        assert!(!valid);
        assert!(matches!(
            patt,
            PattLoclTy::Or {
                fst: box PattLoclTy::Invalid(..),
                ..
            }
        ));
    }

    #[test]
    fn test_validate_ty_shadow() {
        let name = PattName::As {
            lbl: "x".to_string(),
            patt: Box::new(PattName::Name {
                namespace: Namespace::Root,
                name: PattString::Exactly("Classy".to_string()),
            }),
        };
        let hd = PattLoclTy::As {
            lbl: "x".to_string(),
            patt: Box::new(PattLoclTy::Dynamic),
        };

        let tl = Box::new(Params::Nil);
        let params = Box::new(Params::Cons { hd, tl });
        let mut patt = PattLoclTy::Apply { name, params };
        let valid = patt.validate(&mut ValidationEnv::default());
        assert!(!valid);
        assert!(matches!(
            patt,
            PattLoclTy::Apply {
                params: box Params::Cons {
                    hd: PattLoclTy::Invalid(..),
                    ..
                },
                ..
            }
        ))
    }

    #[test]
    fn test_validate_ty_mismatch() {
        let name = PattName::As {
            lbl: "x".to_string(),
            patt: Box::new(PattName::Name {
                namespace: Namespace::Root,
                name: PattString::Exactly("Classy".to_string()),
            }),
        };
        let hd = PattLoclTy::Dynamic;

        let tl = Box::new(Params::Nil);
        let params = Box::new(Params::Cons { hd, tl });
        let fst = Box::new(PattLoclTy::Apply { name, params });
        let snd = Box::new(PattLoclTy::As {
            lbl: "x".to_string(),
            patt: Box::new(PattLoclTy::Dynamic),
        });
        let mut patt = PattLoclTy::Or { fst, snd };
        let valid = patt.validate(&mut ValidationEnv::default());
        assert!(!valid);
        assert!(matches!(
            patt,
            PattLoclTy::Or {
                snd: box PattLoclTy::Invalid(..),
                ..
            }
        ));
    }
}
