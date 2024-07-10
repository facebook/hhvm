// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use parser_core_types::syntax_error;

use crate::gen::experimental_features::FeatureName;
use crate::gen::experimental_features::FeatureName::*;
use crate::gen::experimental_features::FeatureStatus;
use crate::gen::experimental_features::FeatureStatus::*;
use crate::namespace_env::Mode;
use crate::parser_options::ParserOptions;

impl FeatureName {
    fn get_feature_status(&self) -> FeatureStatus {
        match self {
            UnionIntersectionTypeHints => Unstable,
            ClassLevelWhere => Unstable,
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

    // Experimental features with an ongoing release should be allowed by the
    // runtime, but not the typechecker
    pub fn can_use(
        &self,
        po: &ParserOptions,
        mode: &Mode,
        active_unstable_features: &hash::HashSet<FeatureName>,
    ) -> bool {
        (match self {
            UnionIntersectionTypeHints => po.union_intersection_type_hints,
            ClassLevelWhere => po.enable_class_level_where_clauses,
            _ => false,
        }) || active_unstable_features.contains(self)
            || (matches!(self.get_feature_status(), OngoingRelease)
                && matches!(mode, Mode::ForCodegen))
    }

    pub fn enable(
        &self,
        po: &ParserOptions,
        is_hhi: bool,
        active_unstable_features: &mut hash::HashSet<FeatureName>,
        text: &str,
        error: &mut dyn FnMut(syntax_error::Error),
    ) {
        if po.allow_unstable_features
            || is_hhi
            || !matches!(self.get_feature_status(), FeatureStatus::Unstable)
        {
            active_unstable_features.insert(self.clone());
        } else {
            error(syntax_error::cannot_enable_unstable_feature(
                format!("{} is unstable and unstable features are disabled", text).as_str(),
            ))
        }
    }
}
