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
use standard::StandardProtocol;
use standard::TypeName;
use standard::TypeUri;
use type_::Type;
use type_rep::ProtocolUnion;

pub trait GetDummyAny {
    fn get_dummy_any(dummy_data: &str) -> Self;
    fn get_raw_data(&self) -> &Vec<u8>;
}

impl GetDummyAny for Any {
    fn get_dummy_any(dummy_data: &str) -> any::Any {
        any::Any {
            r#type: Type {
                name: TypeName::structType(TypeUri::uri(dummy_data.to_string())),
                ..Default::default()
            },
            protocol: ProtocolUnion::standard(StandardProtocol::Compact),
            data: dummy_data.into(),
            ..Default::default()
        }
    }
    fn get_raw_data(&self) -> &Vec<u8> {
        &self.data
    }
}
