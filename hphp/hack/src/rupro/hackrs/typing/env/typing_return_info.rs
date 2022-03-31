// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use ty::local::Ty;
use ty::reason::Reason;

/// Return type information as gathered from explicit hints or inferred.
#[derive(Debug, Clone)]
pub struct TypingReturnInfo<R: Reason> {
    /// Return type itself. Awaitable<_> is represented explicitly, not
    /// stripped. In the case of lambdas without an explicit return type
    /// hint, the return type is determined by context or is a type variable
    /// that may get resolved when checking the body of the lambda.
    pub return_type: Ty<R>,
}

impl<R: Reason> TypingReturnInfo<R> {
    pub fn new(return_type: Ty<R>) -> Self {
        Self { return_type }
    }

    pub fn placeholder() -> Self {
        Self {
            return_type: Ty::any(R::none()),
        }
    }
}
