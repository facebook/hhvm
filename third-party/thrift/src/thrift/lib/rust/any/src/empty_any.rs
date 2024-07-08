/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

use any::Any;
use standard::TypeName;
use standard::TypeUri;
use type_rep::TypeStruct;

pub fn get_empty_any_struct(want_type: &str) -> Any {
    Any {
        r#type: TypeStruct {
            name: TypeName::structType(TypeUri::uri(want_type.into())),
            ..Default::default()
        },
        ..Default::default()
    }
}
