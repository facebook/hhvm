// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use std::str::FromStr;

use parser_core_types::syntax_error;

use crate::gen::experimental_features::FeatureName;
use crate::gen::experimental_features::FeatureName::*;
use crate::gen::experimental_features::FeatureStatus;
use crate::gen::experimental_features::FeatureStatus::*;
use crate::namespace_env::Mode;
use crate::parser_options::ParserOptions;

impl FeatureName {
    fn get_feature_status_deprecated(&self) -> FeatureStatus {
        match self {
            UnionIntersectionTypeHints => Unstable,
            ExpressionTrees => Unstable,
            Readonly => Preview,
            ModuleReferences => Unstable,
            ContextAliasDeclaration => Unstable,
            ContextAliasDeclarationShort => Preview,
            TypeConstMultipleBounds => Preview,
            TypeConstSuperBound => Unstable,
            ClassConstDefault => Migration,
            TypeRefinements => OngoingRelease,
            MethodTraitDiamond => OngoingRelease,
            UpcastExpression => Unstable,
            RequireClass => OngoingRelease,
            NewtypeSuperBounds => Unstable,
            ExpressionTreeBlocks => OngoingRelease,
            Package => OngoingRelease,
            CaseTypes => Preview,
            ModuleLevelTraits => OngoingRelease,
            ModuleLevelTraitsExtensions => OngoingRelease,
            TypedLocalVariables => Preview,
            PipeAwait => Preview,
            MatchStatements => Unstable,
            StrictSwitch => Unstable,
            ClassType => Unstable,
            FunctionReferences => Unstable,
            FunctionTypeOptionalParams => OngoingRelease,
            ExpressionTreeMap => OngoingRelease,
            ExpressionTreeNest => Preview,
            SealedMethods => Unstable,
            AwaitInSplice => Preview,
        }
    }

    pub fn parse_experimental_feature(
        (name, status): &(String, String),
    ) -> Result<(FeatureName, FeatureStatus), anyhow::Error> {
        let n = FeatureName::from_str(name).map_err(|_| ExperimentalFeatureError {
            kind: "name".to_string(),
            bad_name: name.to_string(),
        })?;
        let s = FeatureStatus::from_str(status).map_err(|_| ExperimentalFeatureError {
            kind: "status".to_string(),
            bad_name: status.to_string(),
        })?;
        let hard_coded_status = n.get_feature_status_deprecated();
        // For now, force the config to be consistent with the hard coded status.
        if s == hard_coded_status {
            Ok((n, s))
        } else {
            Err(anyhow::Error::new(ExperimentalFeatureError {
                kind: "mismatch".to_string(),
                bad_name: format!(
                    "for feature {}: {} in config must be {:?} during experimental feature config roll-out",
                    name, status, hard_coded_status
                ),
            }))
        }
    }

    fn get_feature_status(&self, po: &ParserOptions) -> FeatureStatus {
        if po.use_legacy_experimental_feature_config {
            self.get_feature_status_deprecated()
        } else {
            po.experimental_features
                .iter()
                .find_map(|(name, status)| if name == self { Some(status) } else { None })
                .unwrap_or_else(|| get_unspecified_feature(po))
                .clone()
        }
    }

    // Experimental features with an ongoing release should be allowed by the
    // runtime, but not the typechecker
    pub fn can_use(
        &self,
        po: &ParserOptions,
        mode: &Mode,
        active_experimental_features: &hash::HashSet<FeatureName>,
    ) -> bool {
        (match self {
            UnionIntersectionTypeHints => po.union_intersection_type_hints,
            _ => false,
        }) || active_experimental_features.contains(self)
            || (matches!(mode, Mode::ForCodegen)
                && matches!(self.get_feature_status(po), OngoingRelease))
    }

    pub fn enable(
        &self,
        po: &ParserOptions,
        is_hhi: bool,
        active_experimental_features: &mut hash::HashSet<FeatureName>,
        text: &str,
        error: &mut dyn FnMut(syntax_error::Error),
    ) {
        if po.allow_unstable_features
            || is_hhi
            || !matches!(self.get_feature_status(po), FeatureStatus::Unstable)
        {
            active_experimental_features.insert(self.clone());
        } else {
            error(syntax_error::cannot_enable_unstable_feature(
                format!("{} is unstable and unstable features are disabled", text).as_str(),
            ))
        }
    }
}

fn get_unspecified_feature(po: &ParserOptions) -> &FeatureStatus {
    if po.consider_unspecified_experimental_features_released {
        &OngoingRelease
    } else {
        &Unstable
    }
}

#[derive(Debug)]
struct ExperimentalFeatureError {
    // the name of the experimental feature with the error
    bad_name: String,
    // a description of the error
    kind: String,
}

impl std::fmt::Display for ExperimentalFeatureError {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        write!(
            f,
            "Invalid experimental feature {}: {}",
            self.kind, self.bad_name
        )
    }
}

impl std::error::Error for ExperimentalFeatureError {}
