// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use hhbc_gen::ImmType;
use hhbc_gen::Inputs;
use hhbc_gen::InstrFlags;
use hhbc_gen::OpcodeData;
use hhbc_gen::Outputs;

pub fn test_opcodes() -> Vec<OpcodeData> {
    vec![
        OpcodeData {
            name: "TestZeroImm",
            immediates: vec![],
            inputs: Inputs::NOV,
            outputs: Outputs::NOV,
            flags: InstrFlags::NF,
        },
        OpcodeData {
            name: "TestOneImm",
            immediates: vec![("str1", ImmType::SA)],
            inputs: Inputs::NOV,
            outputs: Outputs::NOV,
            flags: InstrFlags::NF,
        },
        OpcodeData {
            name: "TestTwoImm",
            immediates: vec![("str1", ImmType::SA), ("str2", ImmType::SA)],
            inputs: Inputs::NOV,
            outputs: Outputs::NOV,
            flags: InstrFlags::NF,
        },
        OpcodeData {
            name: "TestThreeImm",
            immediates: vec![
                ("str1", ImmType::SA),
                ("str2", ImmType::SA),
                ("str3", ImmType::SA),
            ],
            inputs: Inputs::NOV,
            outputs: Outputs::NOV,
            flags: InstrFlags::NF,
        },
        // --------------------------------------------------
        OpcodeData {
            name: "TestAA",
            immediates: vec![("arr1", ImmType::AA)],
            inputs: Inputs::NOV,
            outputs: Outputs::NOV,
            flags: InstrFlags::NF,
        },
        OpcodeData {
            name: "TestARR",
            immediates: vec![("arr1", ImmType::ARR(Box::new(ImmType::SA)))],
            inputs: Inputs::NOV,
            outputs: Outputs::NOV,
            flags: InstrFlags::NF,
        },
        OpcodeData {
            name: "TestBA",
            immediates: vec![("target1", ImmType::BA)],
            inputs: Inputs::NOV,
            outputs: Outputs::NOV,
            flags: InstrFlags::CF | InstrFlags::TF,
        },
        OpcodeData {
            name: "TestBA2",
            immediates: vec![("target1", ImmType::BA2)],
            inputs: Inputs::NOV,
            outputs: Outputs::NOV,
            flags: InstrFlags::CF | InstrFlags::TF,
        },
        OpcodeData {
            name: "TestBLA",
            immediates: vec![("targets", ImmType::BLA)],
            inputs: Inputs::NOV,
            outputs: Outputs::NOV,
            flags: InstrFlags::CF | InstrFlags::TF,
        },
        OpcodeData {
            name: "TestDA",
            immediates: vec![("dbl1", ImmType::DA)],
            inputs: Inputs::NOV,
            outputs: Outputs::NOV,
            flags: InstrFlags::NF,
        },
        OpcodeData {
            name: "TestFCA",
            immediates: vec![("fca", ImmType::FCA)],
            inputs: Inputs::NOV,
            outputs: Outputs::NOV,
            flags: InstrFlags::CF,
        },
        OpcodeData {
            name: "TestI64A",
            immediates: vec![("arg1", ImmType::I64A)],
            inputs: Inputs::NOV,
            outputs: Outputs::NOV,
            flags: InstrFlags::NF,
        },
        OpcodeData {
            name: "TestIA",
            immediates: vec![("iter1", ImmType::IA)],
            inputs: Inputs::NOV,
            outputs: Outputs::NOV,
            flags: InstrFlags::NF,
        },
        OpcodeData {
            name: "TestILA",
            immediates: vec![("loc1", ImmType::ILA)],
            inputs: Inputs::NOV,
            outputs: Outputs::NOV,
            flags: InstrFlags::NF,
        },
        OpcodeData {
            name: "TestITA",
            immediates: vec![("ita", ImmType::ITA)],
            inputs: Inputs::NOV,
            outputs: Outputs::NOV,
            flags: InstrFlags::CF,
        },
        OpcodeData {
            name: "TestIVA",
            immediates: vec![("arg1", ImmType::IVA)],
            inputs: Inputs::NOV,
            outputs: Outputs::NOV,
            flags: InstrFlags::NF,
        },
        OpcodeData {
            name: "TestKA",
            immediates: vec![("mkey", ImmType::KA)],
            inputs: Inputs::NOV,
            outputs: Outputs::NOV,
            flags: InstrFlags::NF,
        },
        OpcodeData {
            name: "TestLA",
            immediates: vec![("loc1", ImmType::LA)],
            inputs: Inputs::NOV,
            outputs: Outputs::NOV,
            flags: InstrFlags::NF,
        },
        OpcodeData {
            name: "TestLAR",
            immediates: vec![("locrange", ImmType::LAR)],
            inputs: Inputs::NOV,
            outputs: Outputs::NOV,
            flags: InstrFlags::CF,
        },
        OpcodeData {
            name: "TestNLA",
            immediates: vec![("nloc1", ImmType::NLA)],
            inputs: Inputs::NOV,
            outputs: Outputs::NOV,
            flags: InstrFlags::NF,
        },
        OpcodeData {
            name: "TestOA",
            immediates: vec![("subop1", ImmType::OA("OaSubType"))],
            inputs: Inputs::NOV,
            outputs: Outputs::NOV,
            flags: InstrFlags::NF,
        },
        OpcodeData {
            name: "TestOAL",
            immediates: vec![("subop1", ImmType::OAL("OaSubType"))],
            inputs: Inputs::NOV,
            outputs: Outputs::NOV,
            flags: InstrFlags::NF,
        },
        OpcodeData {
            name: "TestRATA",
            immediates: vec![("rat", ImmType::RATA)],
            inputs: Inputs::NOV,
            outputs: Outputs::NOV,
            flags: InstrFlags::NF,
        },
        OpcodeData {
            name: "TestSA",
            immediates: vec![("str1", ImmType::SA)],
            inputs: Inputs::NOV,
            outputs: Outputs::NOV,
            flags: InstrFlags::NF,
        },
        OpcodeData {
            name: "TestSLA",
            immediates: vec![("targets", ImmType::SLA)],
            inputs: Inputs::NOV,
            outputs: Outputs::NOV,
            flags: InstrFlags::CF | InstrFlags::TF,
        },
        OpcodeData {
            name: "TestVSA",
            immediates: vec![("keys", ImmType::VSA)],
            inputs: Inputs::NOV,
            outputs: Outputs::NOV,
            flags: InstrFlags::NF,
        },
    ]
}
