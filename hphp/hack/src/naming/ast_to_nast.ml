(**
 * Copyright (c) 2019, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)


module AstToNastEnv = struct
 module AastAnnotations = Nast.Annotations
 let get_expr_annotation (p: Ast.pos) = p
 let env_annotation = ()
 let funcbody_annotation = Nast.BodyNamingAnnotation.NamedWithUnsafeBlocks
end

include Ast_to_aast.MapAstToAast(AstToNastEnv)
