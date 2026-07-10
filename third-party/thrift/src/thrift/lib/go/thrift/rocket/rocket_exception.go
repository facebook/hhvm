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

package rocket

import (
	"encoding/json"

	"github.com/facebook/fbthrift/thrift/lib/go/thrift/types"
	"github.com/facebook/fbthrift/thrift/lib/thrift/rpcmetadata"
)

// RocketExceptionType is the type of a Thrift Rocket exception.
type RocketExceptionType int16

const (
	RocketExceptionUnknown         RocketExceptionType = 0
	RocketExceptionDeclared        RocketExceptionType = 1
	RocketExceptionAppUnknown      RocketExceptionType = 2
	RocketExceptionAny             RocketExceptionType = 3
	RocketExceptionDeprecatedProxy RocketExceptionType = 4
)

func (e RocketExceptionType) String() string {
	switch e {
	case RocketExceptionUnknown:
		return "UnknownException"
	case RocketExceptionDeclared:
		return "DeclaredException"
	case RocketExceptionAppUnknown:
		return "AppUnknownException"
	case RocketExceptionAny:
		return "AnyException"
	case RocketExceptionDeprecatedProxy:
		return "DEPRECATEDProxyException"
	}
	panic("unreachable")
}

// MarshalJSON implements json.Marshaler.
func (e RocketExceptionType) MarshalJSON() ([]byte, error) {
	return json.Marshal(e.String())
}

// RocketException is a Thrift Rocket exception.
type RocketException struct {
	Name          string
	What          string
	ExceptionType RocketExceptionType
	Kind          rpcmetadata.ErrorKind
	Blame         rpcmetadata.ErrorBlame
	Safety        rpcmetadata.ErrorSafety
	// Optional serialized exception (set for "Any" and "DeprecatedProxy" exceptions)
	SerializedException []byte
}

var _ error = (*RocketException)(nil)

func newRocketException(exception *rpcmetadata.PayloadExceptionMetadataBase) *RocketException {
	result := &RocketException{
		Name:          "unknown",
		What:          "unknown",
		ExceptionType: RocketExceptionUnknown,
		Kind:          rpcmetadata.ErrorKind_UNSPECIFIED,
		Blame:         rpcmetadata.ErrorBlame_UNSPECIFIED,
		Safety:        rpcmetadata.ErrorSafety_UNSPECIFIED,
	}
	if exception.NameUTF8 != nil {
		result.Name = *exception.NameUTF8
	}
	if exception.WhatUTF8 != nil {
		result.What = *exception.WhatUTF8
	}

	metadata := exception.Metadata
	if metadata == nil {
		// No metadata - we are done here.
		return result
	}

	var classification *rpcmetadata.ErrorClassification
	if metadata.DeclaredException != nil {
		result.ExceptionType = RocketExceptionDeclared
		classification = metadata.DeclaredException.ErrorClassification
	} else if metadata.AppUnknownException != nil {
		result.ExceptionType = RocketExceptionAppUnknown
		classification = metadata.AppUnknownException.ErrorClassification
	} else if metadata.AnyException != nil {
		result.ExceptionType = RocketExceptionAny
	} else if metadata.DEPRECATEDProxyException != nil {
		result.ExceptionType = RocketExceptionDeprecatedProxy
	}
	if classification != nil {
		if classification.Kind != nil {
			result.Kind = classification.GetKind()
		}
		if classification.Blame != nil {
			result.Blame = classification.GetBlame()
		}
		if classification.Safety != nil {
			result.Safety = classification.GetSafety()
		}
	}
	return result
}

func newUnknownPayloadExceptionMetadataBase(name string, what string) *rpcmetadata.PayloadExceptionMetadataBase {
	return newPayloadExceptionMetadataBaseV1(
		name,
		what,
		RocketExceptionUnknown,
		rpcmetadata.ErrorKind_TRANSIENT,
		rpcmetadata.ErrorBlame_SERVER,
		rpcmetadata.ErrorSafety_SAFE,
	)
}

// newPayloadExceptionMetadataBaseV1 creates a new PayloadExceptionMetadataBase.
func newPayloadExceptionMetadataBaseV1(
	name string,
	what string,
	exType RocketExceptionType,
	kind rpcmetadata.ErrorKind,
	blame rpcmetadata.ErrorBlame,
	safety rpcmetadata.ErrorSafety,
) *rpcmetadata.PayloadExceptionMetadataBase {
	classification := rpcmetadata.NewErrorClassification().
		SetKind(&kind).
		SetBlame(&blame).
		SetSafety(&safety)

	metadata := rpcmetadata.NewPayloadExceptionMetadata()
	switch exType {
	case RocketExceptionDeclared:
		declared := rpcmetadata.NewPayloadDeclaredExceptionMetadata().
			SetErrorClassification(classification)
		metadata.SetDeclaredException(declared)
	case RocketExceptionAppUnknown:
		appUnknown := rpcmetadata.NewPayloadAppUnknownExceptionMetdata().
			SetErrorClassification(classification)
		metadata.SetAppUnknownException(appUnknown)
	case RocketExceptionAny:
		metadata.SetAnyException(rpcmetadata.NewPayloadAnyExceptionMetadata())
	case RocketExceptionDeprecatedProxy:
		metadata.SetDEPRECATEDProxyException(rpcmetadata.NewPayloadProxyExceptionMetadata())
	case RocketExceptionUnknown:
	default:
		panic("unreachable")
	}

	base := rpcmetadata.NewPayloadExceptionMetadataBase().
		SetNameUTF8(&name).
		SetWhatUTF8(&what).
		SetMetadata(metadata)
	return base
}

// NewPayloadExceptionMetadataBaseV2 creates a new PayloadExceptionMetadataBase
// from an error, deriving the name, exception type, and error-classification
// (kind/blame/safety) from the error type's entry in the error registry.
//
// Generated code populates the registry via init() for every Thrift exception
// type:
//   - If err is registered, the metadata is built as a declared exception
//     using the registered TypeName and classification.
//   - Otherwise the metadata is built as an AppUnknown exception with the
//     name "ApplicationException" and UNSPECIFIED classification.
func NewPayloadExceptionMetadataBaseV2(err error) *rpcmetadata.PayloadExceptionMetadataBase {
	name := "ApplicationException"
	exType := RocketExceptionAppUnknown
	kind := rpcmetadata.ErrorKind_UNSPECIFIED
	blame := rpcmetadata.ErrorBlame_UNSPECIFIED
	safety := rpcmetadata.ErrorSafety_UNSPECIFIED

	if spec := types.InternalErrorRegistryGet(err); spec != nil {
		name = spec.TypeName
		exType = RocketExceptionDeclared
		kind = toRPCMetadataErrorKind(spec.Kind)
		blame = toRPCMetadataErrorBlame(spec.Blame)
		safety = toRPCMetadataErrorSafety(spec.Safety)
	}

	return newPayloadExceptionMetadataBaseV1(
		name,
		err.Error(),
		exType,
		kind,
		blame,
		safety,
	)
}

func toRPCMetadataErrorKind(k types.ErrorKind) rpcmetadata.ErrorKind {
	switch k {
	case types.ErrorKindTransient:
		return rpcmetadata.ErrorKind_TRANSIENT
	case types.ErrorKindStateful:
		return rpcmetadata.ErrorKind_STATEFUL
	case types.ErrorKindPermanent:
		return rpcmetadata.ErrorKind_PERMANENT
	default:
		return rpcmetadata.ErrorKind_UNSPECIFIED
	}
}

func toRPCMetadataErrorBlame(b types.ErrorBlame) rpcmetadata.ErrorBlame {
	switch b {
	case types.ErrorBlameServer:
		return rpcmetadata.ErrorBlame_SERVER
	case types.ErrorBlameClient:
		return rpcmetadata.ErrorBlame_CLIENT
	default:
		return rpcmetadata.ErrorBlame_UNSPECIFIED
	}
}

func toRPCMetadataErrorSafety(s types.ErrorSafety) rpcmetadata.ErrorSafety {
	switch s {
	case types.ErrorSafetySafe:
		return rpcmetadata.ErrorSafety_SAFE
	default:
		return rpcmetadata.ErrorSafety_UNSPECIFIED
	}
}

// Error implements the error interface.
func (e *RocketException) Error() string {
	data, err := json.Marshal(e)
	if err != nil {
		panic(err)
	}
	return string(data)
}

// IsDeclared returns true if the exception is a declared exception.
func (e *RocketException) IsDeclared() bool {
	return e.ExceptionType == RocketExceptionDeclared
}
