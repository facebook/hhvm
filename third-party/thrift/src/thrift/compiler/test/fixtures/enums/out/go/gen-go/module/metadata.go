// Autogenerated by Thrift for thrift/compiler/test/fixtures/enums/src/module.thrift
//
// DO NOT EDIT UNLESS YOU ARE SURE THAT YOU KNOW WHAT YOU ARE DOING
//  @generated

package module

import (
    "maps"

    thrift "github.com/facebook/fbthrift/thrift/lib/go/thrift/types"
    metadata "github.com/facebook/fbthrift/thrift/lib/thrift/metadata"
)

// (needed to ensure safety because of naive import list construction)
var _ = thrift.VOID
var _ = maps.Copy[map[int]int, map[int]int]
var _ = metadata.GoUnusedProtection__

// Premade Thrift types
var (
    premadeThriftType_module_Metasyntactic =
        &metadata.ThriftType{
            TEnum:
                &metadata.ThriftEnumType{
                    Name: "module.Metasyntactic",
                },
        }
    premadeThriftType_module_MyEnum1 =
        &metadata.ThriftType{
            TEnum:
                &metadata.ThriftEnumType{
                    Name: "module.MyEnum1",
                },
        }
    premadeThriftType_module_MyEnum2 =
        &metadata.ThriftType{
            TEnum:
                &metadata.ThriftEnumType{
                    Name: "module.MyEnum2",
                },
        }
    premadeThriftType_module_MyEnum3 =
        &metadata.ThriftType{
            TEnum:
                &metadata.ThriftEnumType{
                    Name: "module.MyEnum3",
                },
        }
    premadeThriftType_module_MyEnum4 =
        &metadata.ThriftType{
            TEnum:
                &metadata.ThriftEnumType{
                    Name: "module.MyEnum4",
                },
        }
    premadeThriftType_module_MyBitmaskEnum1 =
        &metadata.ThriftType{
            TEnum:
                &metadata.ThriftEnumType{
                    Name: "module.MyBitmaskEnum1",
                },
        }
    premadeThriftType_module_MyBitmaskEnum2 =
        &metadata.ThriftType{
            TEnum:
                &metadata.ThriftEnumType{
                    Name: "module.MyBitmaskEnum2",
                },
        }
    premadeThriftType_i32 =
        &metadata.ThriftType{
            TPrimitive:
                thrift.Pointerize(metadata.ThriftPrimitiveType_THRIFT_I32_TYPE),
        }
    premadeThriftType_set_i32 =
        &metadata.ThriftType{
            TSet:
                &metadata.ThriftSetType{
                    ValueType: premadeThriftType_i32,
                },
        }
    premadeThriftType_module_SomeStruct =
        &metadata.ThriftType{
            TStruct:
                &metadata.ThriftStructType{
                    Name: "module.SomeStruct",
                },
        }
    premadeThriftType_module_MyStruct =
        &metadata.ThriftType{
            TStruct:
                &metadata.ThriftStructType{
                    Name: "module.MyStruct",
                },
        }
)

var premadeThriftTypesMap = func() map[string]*metadata.ThriftType {
    fbthriftThriftTypesMap := make(map[string]*metadata.ThriftType)
    fbthriftThriftTypesMap["module.Metasyntactic"] = premadeThriftType_module_Metasyntactic
    fbthriftThriftTypesMap["module.MyEnum1"] = premadeThriftType_module_MyEnum1
    fbthriftThriftTypesMap["module.MyEnum2"] = premadeThriftType_module_MyEnum2
    fbthriftThriftTypesMap["module.MyEnum3"] = premadeThriftType_module_MyEnum3
    fbthriftThriftTypesMap["module.MyEnum4"] = premadeThriftType_module_MyEnum4
    fbthriftThriftTypesMap["module.MyBitmaskEnum1"] = premadeThriftType_module_MyBitmaskEnum1
    fbthriftThriftTypesMap["module.MyBitmaskEnum2"] = premadeThriftType_module_MyBitmaskEnum2
    fbthriftThriftTypesMap["i32"] = premadeThriftType_i32
    fbthriftThriftTypesMap["module.SomeStruct"] = premadeThriftType_module_SomeStruct
    fbthriftThriftTypesMap["module.MyStruct"] = premadeThriftType_module_MyStruct
    return fbthriftThriftTypesMap
}()

var structMetadatas = func() []*metadata.ThriftStruct {
    fbthriftResults := make([]*metadata.ThriftStruct, 0)
    func() {
        fbthriftResults = append(fbthriftResults,
            &metadata.ThriftStruct{
                Name:    "module.SomeStruct",
                IsUnion: false,
                Fields:  []*metadata.ThriftField{
                    &metadata.ThriftField{
                        Id:         1,
                        Name:       "reasonable",
                        IsOptional: false,
                        Type:       premadeThriftType_module_Metasyntactic,
                    },
                    &metadata.ThriftField{
                        Id:         2,
                        Name:       "fine",
                        IsOptional: false,
                        Type:       premadeThriftType_module_Metasyntactic,
                    },
                    &metadata.ThriftField{
                        Id:         3,
                        Name:       "questionable",
                        IsOptional: false,
                        Type:       premadeThriftType_module_Metasyntactic,
                    },
                    &metadata.ThriftField{
                        Id:         4,
                        Name:       "tags",
                        IsOptional: false,
                        Type:       premadeThriftType_set_i32,
                    },
                },
            },
        )
    }()
    func() {
        fbthriftResults = append(fbthriftResults,
            &metadata.ThriftStruct{
                Name:    "module.MyStruct",
                IsUnion: false,
                Fields:  []*metadata.ThriftField{
                    &metadata.ThriftField{
                        Id:         1,
                        Name:       "me2_3",
                        IsOptional: false,
                        Type:       premadeThriftType_module_MyEnum2,
                    },
                    &metadata.ThriftField{
                        Id:         2,
                        Name:       "me3_n3",
                        IsOptional: false,
                        Type:       premadeThriftType_module_MyEnum3,
                    },
                    &metadata.ThriftField{
                        Id:         4,
                        Name:       "me1_t1",
                        IsOptional: false,
                        Type:       premadeThriftType_module_MyEnum1,
                    },
                    &metadata.ThriftField{
                        Id:         6,
                        Name:       "me1_t2",
                        IsOptional: false,
                        Type:       premadeThriftType_module_MyEnum1,
                    },
                },
            },
        )
    }()
    return fbthriftResults
}()

var exceptionMetadatas = func() []*metadata.ThriftException {
    fbthriftResults := make([]*metadata.ThriftException, 0)
    return fbthriftResults
}()

var enumMetadatas = func() []*metadata.ThriftEnum {
    fbthriftResults := make([]*metadata.ThriftEnum, 0)
    fbthriftResults = append(fbthriftResults,
        &metadata.ThriftEnum{
            Name:     "module.Metasyntactic",
            Elements: map[int32]string{
                1: "FOO",
                2: "BAR",
                3: "BAZ",
                4: "BAX",
            },
        },
    )
    fbthriftResults = append(fbthriftResults,
        &metadata.ThriftEnum{
            Name:     "module.MyEnum1",
            Elements: map[int32]string{
                0: "ME1_0",
                1: "ME1_1",
                2: "ME1_2",
                3: "ME1_3",
                5: "ME1_5",
                6: "ME1_6",
            },
        },
    )
    fbthriftResults = append(fbthriftResults,
        &metadata.ThriftEnum{
            Name:     "module.MyEnum2",
            Elements: map[int32]string{
                0: "ME2_0",
                1: "ME2_1",
                2: "ME2_2",
            },
        },
    )
    fbthriftResults = append(fbthriftResults,
        &metadata.ThriftEnum{
            Name:     "module.MyEnum3",
            Elements: map[int32]string{
                0: "ME3_0",
                1: "ME3_1",
                -2: "ME3_N2",
                -1: "ME3_N1",
                9: "ME3_9",
                10: "ME3_10",
            },
        },
    )
    fbthriftResults = append(fbthriftResults,
        &metadata.ThriftEnum{
            Name:     "module.MyEnum4",
            Elements: map[int32]string{
                2147483645: "ME4_A",
                2147483646: "ME4_B",
                2147483647: "ME4_C",
                -2147483648: "ME4_D",
            },
        },
    )
    fbthriftResults = append(fbthriftResults,
        &metadata.ThriftEnum{
            Name:     "module.MyBitmaskEnum1",
            Elements: map[int32]string{
                1: "ONE",
                2: "TWO",
                4: "FOUR",
            },
        },
    )
    fbthriftResults = append(fbthriftResults,
        &metadata.ThriftEnum{
            Name:     "module.MyBitmaskEnum2",
            Elements: map[int32]string{
                1: "ONE",
                2: "TWO",
                4: "FOUR",
            },
        },
    )
    return fbthriftResults
}()

var serviceMetadatas = func() []*metadata.ThriftService {
    fbthriftResults := make([]*metadata.ThriftService, 0)
    return fbthriftResults
}()

// Thrift metadata for this package, as well as all of its recursive imports.
var packageThriftMetadata = func() *metadata.ThriftMetadata {
    allEnumsMap := make(map[string]*metadata.ThriftEnum)
    allStructsMap := make(map[string]*metadata.ThriftStruct)
    allExceptionsMap := make(map[string]*metadata.ThriftException)
    allServicesMap := make(map[string]*metadata.ThriftService)

    // Add enum metadatas from the current program...
    for _, enumMetadata := range enumMetadatas {
        allEnumsMap[enumMetadata.GetName()] = enumMetadata
    }
    // Add struct metadatas from the current program...
    for _, structMetadata := range structMetadatas {
        allStructsMap[structMetadata.GetName()] = structMetadata
    }
    // Add exception metadatas from the current program...
    for _, exceptionMetadata := range exceptionMetadatas {
        allExceptionsMap[exceptionMetadata.GetName()] = exceptionMetadata
    }
    // Add service metadatas from the current program...
    for _, serviceMetadata := range serviceMetadatas {
        allServicesMap[serviceMetadata.GetName()] = serviceMetadata
    }

    // Obtain Thrift metadatas from recursively included programs...
    var recursiveThriftMetadatas []*metadata.ThriftMetadata

    // ...now merge metadatas from recursively included programs.
    for _, thriftMetadata := range recursiveThriftMetadatas {
        maps.Copy(allEnumsMap, thriftMetadata.GetEnums())
        maps.Copy(allStructsMap, thriftMetadata.GetStructs())
        maps.Copy(allExceptionsMap, thriftMetadata.GetExceptions())
        maps.Copy(allServicesMap, thriftMetadata.GetServices())
    }

    return metadata.NewThriftMetadata().
        SetEnums(allEnumsMap).
        SetStructs(allStructsMap).
        SetExceptions(allExceptionsMap).
        SetServices(allServicesMap)
}()

// GetMetadataThriftType (INTERNAL USE ONLY).
// Returns metadata ThriftType for a given full type name.
func GetMetadataThriftType(fullName string) *metadata.ThriftType {
    return premadeThriftTypesMap[fullName]
}

// GetThriftMetadata returns complete Thrift metadata for current and imported packages.
func GetThriftMetadata() *metadata.ThriftMetadata {
    return packageThriftMetadata
}

// GetThriftMetadataForService returns Thrift metadata for the given service.
func GetThriftMetadataForService(scopedServiceName string) *metadata.ThriftMetadata {
    allServicesMap := packageThriftMetadata.GetServices()
    relevantServicesMap := make(map[string]*metadata.ThriftService)

    serviceMetadata := allServicesMap[scopedServiceName]
    // Visit and record all recursive parents of the target service.
    for serviceMetadata != nil {
        relevantServicesMap[serviceMetadata.GetName()] = serviceMetadata
        if serviceMetadata.IsSetParent() {
            serviceMetadata = allServicesMap[serviceMetadata.GetParent()]
        } else {
            serviceMetadata = nil
        }
    }

    return metadata.NewThriftMetadata().
        SetEnums(packageThriftMetadata.GetEnums()).
        SetStructs(packageThriftMetadata.GetStructs()).
        SetExceptions(packageThriftMetadata.GetExceptions()).
        SetServices(relevantServicesMap)
}
