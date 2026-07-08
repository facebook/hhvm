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

use std::collections::BTreeMap;

use record::SerializableRecord;
use service_catalog::FunctionQualifier;
use service_catalog::RpcKind;
use service_catalog::SerializableBidirectionalStream;
use service_catalog::SerializableFunctionResponse;
use service_catalog::SerializableRpcInterfaceDefinition;
use service_catalog::SerializableServiceCatalog;
use service_catalog::SerializableStreamingResponse;
use service_catalog_digest_expected_values::DIGEST_CALCULATOR;
use service_catalog_digest_expected_values::DIGEST_RICH_DESCRIPTOR;
use service_catalog_digest_expected_values::DIGEST_RICH_DESCRIPTOR_STRUCTURAL;
use type_system_digest::TypeSystemDigest as _;

use super::*;

fn hex_digest(value: &impl ServiceCatalogDigest) -> String {
    value.digest().iter().map(|b| format!("{b:02x}")).collect()
}

fn hex_digest_with_mode(value: &impl ServiceCatalogDigest, mode: DigestMode) -> String {
    value
        .digest_with_mode(mode)
        .iter()
        .map(|b| format!("{b:02x}"))
        .collect()
}

fn empty_type_system() -> type_system::SerializableTypeSystem {
    type_system::SerializableTypeSystem {
        types: BTreeMap::new(),
        ..Default::default()
    }
}

fn rich_annotation_type_system() -> type_system::SerializableTypeSystem {
    let fields = vec![
        optional_field(1, "label", TypeId::stringType(Default::default())),
        optional_field(2, "enabled", TypeId::boolType(Default::default())),
        optional_field(3, "byteValue", TypeId::byteType(Default::default())),
        optional_field(4, "i16Value", TypeId::i16Type(Default::default())),
        optional_field(5, "i32Value", TypeId::i32Type(Default::default())),
        optional_field(6, "i64Value", TypeId::i64Type(Default::default())),
        optional_field(7, "floatValue", TypeId::floatType(Default::default())),
        optional_field(8, "doubleValue", TypeId::doubleType(Default::default())),
        optional_field(9, "data", TypeId::binaryType(Default::default())),
        optional_field(
            10,
            "tags",
            list_type(TypeId::stringType(Default::default())),
        ),
        optional_field(11, "levels", set_type(TypeId::i32Type(Default::default()))),
        optional_field(
            12,
            "weights",
            map_type(
                TypeId::stringType(Default::default()),
                TypeId::i64Type(Default::default()),
            ),
        ),
    ];
    type_system::SerializableTypeSystem {
        types: BTreeMap::from([(
            "test.com/RichAnnotation".to_owned(),
            type_system::SerializableTypeDefinitionEntry {
                definition: type_system::SerializableTypeDefinition::structDef(
                    type_system::SerializableStructDefinition {
                        fields,
                        isSealed: false,
                        annotations: BTreeMap::new(),
                        ..Default::default()
                    },
                ),
                ..Default::default()
            },
        )]),
        ..Default::default()
    }
}

fn optional_field(id: i16, name: &str, ty: TypeId) -> type_system::SerializableFieldDefinition {
    type_system::SerializableFieldDefinition {
        identity: type_system::FieldIdentity {
            id,
            name: name.to_owned(),
            ..Default::default()
        },
        presence: type_system::PresenceQualifier::OPTIONAL,
        r#type: ty,
        customDefaultPartialRecord: None,
        annotations: BTreeMap::new(),
        ..Default::default()
    }
}

fn list_type(element: TypeId) -> TypeId {
    TypeId::listType(type_id::ListTypeId {
        elementType: Some(Box::new(element)),
        ..Default::default()
    })
}

fn set_type(element: TypeId) -> TypeId {
    TypeId::setType(type_id::SetTypeId {
        elementType: Some(Box::new(element)),
        ..Default::default()
    })
}

fn map_type(key: TypeId, value: TypeId) -> TypeId {
    TypeId::mapType(type_id::MapTypeId {
        keyType: Some(Box::new(key)),
        valueType: Some(Box::new(value)),
        ..Default::default()
    })
}

fn calculator_descriptor() -> ServiceDescriptor {
    let mut descriptor = ServiceDescriptor::new(
        "test.com/Calculator",
        TypeUniverse::Inline(empty_type_system()),
    );
    descriptor.functions = vec![calculator_function("subtract"), calculator_function("add")];
    descriptor
}

fn calculator_function(name: &str) -> Function {
    Function {
        name: name.to_owned(),
        params: vec![
            param(1, "left", TypeId::i32Type(Default::default())),
            param(2, "right", TypeId::i32Type(Default::default())),
        ],
        response_type: Some(TypeId::i32Type(Default::default())),
        ..Default::default()
    }
}

fn param(id: i16, name: &str, type_id: TypeId) -> Parameter {
    Parameter {
        id,
        name: name.to_owned(),
        type_id,
        annotations: AnnotationsMap::new(),
    }
}

fn exception(id: i16, name: &str, type_id: TypeId) -> Exception {
    Exception {
        id,
        name: name.to_owned(),
        type_id,
        annotations: AnnotationsMap::new(),
    }
}

fn rich_descriptor() -> ServiceDescriptor {
    let mut descriptor = ServiceDescriptor::new(
        "test.com/CatalogGolden",
        TypeUniverse::Inline(rich_annotation_type_system()),
    );
    descriptor.annotations = rich_annotations();
    descriptor.functions = vec![
        Function {
            name: "makeSession".to_owned(),
            params: vec![Parameter {
                annotations: rich_annotations(),
                ..param(1, "seed", TypeId::i32Type(Default::default()))
            }],
            qualifier: FunctionQualifier(1),
            created_interaction_uri: Some("test.com/CatalogGoldenSession".to_owned()),
            annotations: rich_annotations(),
            ..Default::default()
        },
        Function {
            name: "observe".to_owned(),
            stream: Some(Stream {
                payload_type: TypeId::i32Type(Default::default()),
                exceptions: Vec::new(),
            }),
            sink: Some(Sink {
                payload_type: TypeId::stringType(Default::default()),
                final_response_type: None,
                client_exceptions: Vec::new(),
                server_exceptions: Vec::new(),
            }),
            rpc_kind: RpcKind(4),
            annotations: rich_annotations(),
            ..Default::default()
        },
        Function {
            name: "upload".to_owned(),
            exceptions: vec![exception(
                1,
                "appError",
                TypeId::stringType(Default::default()),
            )],
            sink: Some(Sink {
                payload_type: TypeId::i32Type(Default::default()),
                final_response_type: Some(TypeId::stringType(Default::default())),
                client_exceptions: Vec::new(),
                server_exceptions: Vec::new(),
            }),
            rpc_kind: RpcKind(3),
            ..Default::default()
        },
        Function {
            name: "notify".to_owned(),
            rpc_kind: RpcKind(1),
            ..Default::default()
        },
    ];
    descriptor.interactions = vec![Interaction {
        uri: "test.com/CatalogGoldenSession".to_owned(),
        functions: vec![Function {
            name: "get".to_owned(),
            response_type: Some(TypeId::i64Type(Default::default())),
            qualifier: FunctionQualifier(2),
            annotations: rich_annotations(),
            ..Default::default()
        }],
        annotations: rich_annotations(),
    }];
    descriptor
}

fn rich_annotations() -> AnnotationsMap {
    BTreeMap::from([(
        "test.com/RichAnnotation".to_owned(),
        SerializableRecord::fieldSetDatum(BTreeMap::from([
            (
                1,
                SerializableRecord::textDatum("runtime\0value".to_owned()),
            ),
            (2, SerializableRecord::boolDatum(true)),
            (3, SerializableRecord::int8Datum(-7)),
            (4, SerializableRecord::int16Datum(-1234)),
            (5, SerializableRecord::int32Datum(123456)),
            (6, SerializableRecord::int64Datum(1234567890123)),
            (7, SerializableRecord::float32Datum(1.25)),
            (8, SerializableRecord::float64Datum(-2.5)),
            (9, SerializableRecord::byteArrayDatum(b"bin\0data".to_vec())),
            (
                10,
                SerializableRecord::listDatum(vec![
                    SerializableRecord::textDatum("alpha".to_owned()),
                    SerializableRecord::textDatum("beta".to_owned()),
                ]),
            ),
            (
                11,
                SerializableRecord::setDatum(vec![
                    SerializableRecord::int32Datum(2),
                    SerializableRecord::int32Datum(1),
                ]),
            ),
            (
                12,
                SerializableRecord::mapDatum(vec![
                    record::SerializableRecordMapEntry {
                        key: SerializableRecord::textDatum("left".to_owned()),
                        value: SerializableRecord::int64Datum(10),
                        ..Default::default()
                    },
                    record::SerializableRecordMapEntry {
                        key: SerializableRecord::textDatum("right".to_owned()),
                        value: SerializableRecord::int64Datum(20),
                        ..Default::default()
                    },
                ]),
            ),
        ])),
    )])
}

fn to_serializable(descriptor: &ServiceDescriptor) -> SerializableServiceCatalog {
    let mut interfaces = BTreeMap::new();
    interfaces.insert(
        descriptor.service_uri.clone(),
        SerializableRpcInterfaceDefinition::serviceDef(
            service_catalog::SerializableServiceDefinition {
                functions: descriptor
                    .functions
                    .iter()
                    .map(to_serializable_function)
                    .collect(),
                baseService: None,
                annotations: descriptor.annotations.clone(),
                ..Default::default()
            },
        ),
    );
    for interaction in &descriptor.interactions {
        interfaces.insert(
            interaction.uri.clone(),
            SerializableRpcInterfaceDefinition::interactionDef(
                service_catalog::SerializableInteractionDefinition {
                    functions: interaction
                        .functions
                        .iter()
                        .map(to_serializable_function)
                        .collect(),
                    annotations: interaction.annotations.clone(),
                    ..Default::default()
                },
            ),
        );
    }

    let types = match &descriptor.type_universe {
        TypeUniverse::Inline(types) => types.clone(),
        TypeUniverse::Digest(_) => empty_type_system(),
    };
    SerializableServiceCatalog {
        typesDigest: types.digest().to_vec(),
        types: Some(types),
        interfaces,
        ..Default::default()
    }
}

fn to_serializable_function(function: &Function) -> service_catalog::SerializableFunction {
    service_catalog::SerializableFunction {
        name: function.name.clone(),
        qualifier: function.qualifier,
        params: function
            .params
            .iter()
            .map(|param| service_catalog::SerializableParameter {
                identity: type_system::FieldIdentity {
                    id: param.id,
                    name: param.name.clone(),
                    ..Default::default()
                },
                r#type: param.type_id.clone(),
                annotations: param.annotations.clone(),
                ..Default::default()
            })
            .collect(),
        response: to_serializable_response(function),
        exceptions: function
            .exceptions
            .iter()
            .map(|ex| service_catalog::SerializableException {
                identity: type_system::FieldIdentity {
                    id: ex.id,
                    name: ex.name.clone(),
                    ..Default::default()
                },
                r#type: ex.type_id.clone(),
                annotations: ex.annotations.clone(),
                ..Default::default()
            })
            .collect(),
        rpcKind: function.rpc_kind,
        annotations: function.annotations.clone(),
        ..Default::default()
    }
}

fn to_serializable_response(function: &Function) -> SerializableFunctionResponse {
    let streaming = match (&function.stream, &function.sink) {
        (Some(stream), Some(sink)) => Some(SerializableStreamingResponse::bidirectionalStream(
            SerializableBidirectionalStream {
                sinkPayloadType: sink.payload_type.clone(),
                streamPayloadType: stream.payload_type.clone(),
                sinkExceptions: sink
                    .client_exceptions
                    .iter()
                    .map(to_serializable_exception)
                    .collect(),
                streamExceptions: stream
                    .exceptions
                    .iter()
                    .map(to_serializable_exception)
                    .collect(),
                ..Default::default()
            },
        )),
        (Some(stream), None) => Some(SerializableStreamingResponse::serverStream(
            service_catalog::SerializableStream {
                payloadType: stream.payload_type.clone(),
                exceptions: stream
                    .exceptions
                    .iter()
                    .map(to_serializable_exception)
                    .collect(),
                ..Default::default()
            },
        )),
        (None, Some(sink)) => Some(SerializableStreamingResponse::clientSink(
            service_catalog::SerializableSink {
                payloadType: sink.payload_type.clone(),
                finalResponseType: sink.final_response_type.clone(),
                clientExceptions: sink
                    .client_exceptions
                    .iter()
                    .map(to_serializable_exception)
                    .collect(),
                serverExceptions: sink
                    .server_exceptions
                    .iter()
                    .map(to_serializable_exception)
                    .collect(),
                ..Default::default()
            },
        )),
        (None, None) => None,
    };
    SerializableFunctionResponse {
        initialResponseType: function.response_type.clone(),
        streaming,
        createsInteraction: function.created_interaction_uri.clone(),
        ..Default::default()
    }
}

fn to_serializable_exception(ex: &Exception) -> service_catalog::SerializableException {
    service_catalog::SerializableException {
        identity: type_system::FieldIdentity {
            id: ex.id,
            name: ex.name.clone(),
            ..Default::default()
        },
        r#type: ex.type_id.clone(),
        annotations: ex.annotations.clone(),
        ..Default::default()
    }
}

#[test]
fn version_constant_exists() {
    assert_eq!(SERVICE_CATALOG_DIGEST_VERSION, 1);
}

#[test]
fn golden_calculator_digest_matches_cpp() {
    let descriptor = calculator_descriptor();
    let catalog = to_serializable(&descriptor);
    let mut out_of_band = catalog.clone();
    out_of_band.types = None;

    assert_eq!(hex_digest(&descriptor), DIGEST_CALCULATOR);
    assert_eq!(hex_digest(&catalog), DIGEST_CALCULATOR);
    assert_eq!(hex_digest(&out_of_band), DIGEST_CALCULATOR);
}

#[test]
fn golden_rich_descriptor_digest_matches_cpp() {
    let descriptor = rich_descriptor();
    let catalog = to_serializable(&descriptor);

    assert_eq!(hex_digest(&descriptor), DIGEST_RICH_DESCRIPTOR);
    assert_eq!(hex_digest(&catalog), DIGEST_RICH_DESCRIPTOR);
}

#[test]
fn golden_rich_descriptor_structural_digest_matches_cpp() {
    let descriptor = rich_descriptor();
    let catalog = to_serializable(&descriptor);
    let mut out_of_band = catalog.clone();
    out_of_band.typesDigest = catalog
        .types
        .as_ref()
        .unwrap()
        .digest_with_mode(DigestMode::Structural)
        .to_vec();
    out_of_band.types = None;

    assert_eq!(
        hex_digest_with_mode(&descriptor, DigestMode::Structural),
        DIGEST_RICH_DESCRIPTOR_STRUCTURAL,
    );
    assert_eq!(
        hex_digest_with_mode(&catalog, DigestMode::Structural),
        DIGEST_RICH_DESCRIPTOR_STRUCTURAL,
    );
    assert_eq!(
        hex_digest_with_mode(&out_of_band, DigestMode::Structural),
        DIGEST_RICH_DESCRIPTOR_STRUCTURAL,
    );
}

#[test]
fn function_and_parameter_order_independent() {
    let descriptor = calculator_descriptor();
    let mut reordered = descriptor.clone();
    reordered.functions.reverse();
    reordered.functions[0].params.reverse();

    assert_eq!(descriptor.digest(), reordered.digest());
}

#[test]
fn structural_mode_ignores_annotations() {
    let mut first = calculator_descriptor();
    first.annotations = BTreeMap::from([(
        "test.com/Annotation".to_owned(),
        SerializableRecord::textDatum("first".to_owned()),
    )]);
    let mut second = calculator_descriptor();
    second.annotations = BTreeMap::from([(
        "test.com/Annotation".to_owned(),
        SerializableRecord::textDatum("second".to_owned()),
    )]);

    assert_ne!(first.digest(), second.digest());
    assert_eq!(
        first.digest_with_mode(DigestMode::Structural),
        second.digest_with_mode(DigestMode::Structural),
    );
}
