/**
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional
 * directory.
 *
 **
 *
 * THIS FILE IS @generated; DO NOT EDIT IT
 * To regenerate this file, run
 *
 *   buck run //hphp/hack/src:generate_full_fidelity
 *
 **
 *
 */

/**
 * HTML5 special entity decoding
 *
 * HHVM decodes certain HTML entities present in input strings before
 * generating bytecode. In order to generate bytecode identical to HHVM's,
 * this module performs the same HTML entity decoding as HHVM.
 * Mimics: zend-html.cpp
 * The list of entities tested was taken from
 * https://dev.w3.org/html5/html-author/charref on 09/27/2017.
 */
pub fn decode(s: &[u8]) -> Option<&'static [u8]> {
    match s {
        //"bsim" => "∽"
        &[0x62, 0x73, 0x69, 0x6d, ] => Some(&[0xe2, 0x88, 0xbd, ]),
        //"sscue" => "≽"
        &[0x73, 0x73, 0x63, 0x75, 0x65, ] => Some(&[0xe2, 0x89, 0xbd, ]),
        //"becaus" => "∵"
        &[0x62, 0x65, 0x63, 0x61, 0x75, 0x73, ] => Some(&[0xe2, 0x88, 0xb5, ]),
        //"nexist" => "∄"
        &[0x6e, 0x65, 0x78, 0x69, 0x73, 0x74, ] => Some(&[0xe2, 0x88, 0x84, ]),
        //"Atilde" => "Ã"
        &[0x41, 0x74, 0x69, 0x6c, 0x64, 0x65, ] => Some(&[0xc3, 0x83, ]),
        //"emsp" => " "
        &[0x65, 0x6d, 0x73, 0x70, ] => Some(&[0xe2, 0x80, 0x83, ]),
        //"nabla" => "∇"
        &[0x6e, 0x61, 0x62, 0x6c, 0x61, ] => Some(&[0xe2, 0x88, 0x87, ]),
        //"lang" => "〈"
        &[0x6c, 0x61, 0x6e, 0x67, ] => Some(&[0xe2, 0x8c, 0xa9, ]),
        //"Ugrave" => "Ù"
        &[0x55, 0x67, 0x72, 0x61, 0x76, 0x65, ] => Some(&[0xc3, 0x99, ]),
        //"hearts" => "♥"
        &[0x68, 0x65, 0x61, 0x72, 0x74, 0x73, ] => Some(&[0xe2, 0x99, 0xa5, ]),
        //"oplus" => "⊕"
        &[0x6f, 0x70, 0x6c, 0x75, 0x73, ] => Some(&[0xe2, 0x8a, 0x95, ]),
        //"le" => "≤"
        &[0x6c, 0x65, ] => Some(&[0xe2, 0x89, 0xa4, ]),
        //"wreath" => "≀"
        &[0x77, 0x72, 0x65, 0x61, 0x74, 0x68, ] => Some(&[0xe2, 0x89, 0x80, ]),
        //"kappa" => "κ"
        &[0x6b, 0x61, 0x70, 0x70, 0x61, ] => Some(&[0xce, 0xba, ]),
        //"lrm" => "‎"
        &[0x6c, 0x72, 0x6d, ] => Some(&[0xe2, 0x80, 0x8e, ]),
        //"OElig" => "Œ"
        &[0x4f, 0x45, 0x6c, 0x69, 0x67, ] => Some(&[0xc5, 0x92, ]),
        //"prod" => "∏"
        &[0x70, 0x72, 0x6f, 0x64, ] => Some(&[0xe2, 0x88, 0x8f, ]),
        //"npr" => "⊀"
        &[0x6e, 0x70, 0x72, ] => Some(&[0xe2, 0x8a, 0x80, ]),
        //"notin" => "∉"
        &[0x6e, 0x6f, 0x74, 0x69, 0x6e, ] => Some(&[0xe2, 0x88, 0x89, ]),
        //"rsaquo" => "›"
        &[0x72, 0x73, 0x61, 0x71, 0x75, 0x6f, ] => Some(&[0xe2, 0x80, 0xba, ]),
        //"upsilon" => "υ"
        &[0x75, 0x70, 0x73, 0x69, 0x6c, 0x6f, 0x6e, ] => Some(&[0xcf, 0x85, ]),
        //"lg" => "≶"
        &[0x6c, 0x67, ] => Some(&[0xe2, 0x89, 0xb6, ]),
        //"trade" => "™"
        &[0x74, 0x72, 0x61, 0x64, 0x65, ] => Some(&[0xe2, 0x84, 0xa2, ]),
        //"ape" => "≊"
        &[0x61, 0x70, 0x65, ] => Some(&[0xe2, 0x89, 0x8a, ]),
        //"bdquo" => "„"
        &[0x62, 0x64, 0x71, 0x75, 0x6f, ] => Some(&[0xe2, 0x80, 0x9e, ]),
        //"theta" => "θ"
        &[0x74, 0x68, 0x65, 0x74, 0x61, ] => Some(&[0xce, 0xb8, ]),
        //"ldquo" => "“"
        &[0x6c, 0x64, 0x71, 0x75, 0x6f, ] => Some(&[0xe2, 0x80, 0x9c, ]),
        //"Yuml" => "Ÿ"
        &[0x59, 0x75, 0x6d, 0x6c, ] => Some(&[0xc5, 0xb8, ]),
        //"scaron" => "š"
        &[0x73, 0x63, 0x61, 0x72, 0x6f, 0x6e, ] => Some(&[0xc5, 0xa1, ]),
        //"permil" => "‰"
        &[0x70, 0x65, 0x72, 0x6d, 0x69, 0x6c, ] => Some(&[0xe2, 0x80, 0xb0, ]),
        //"xi" => "ξ"
        &[0x78, 0x69, ] => Some(&[0xce, 0xbe, ]),
        //"rsquo" => "’"
        &[0x72, 0x73, 0x71, 0x75, 0x6f, ] => Some(&[0xe2, 0x80, 0x99, ]),
        //"clubs" => "♣"
        &[0x63, 0x6c, 0x75, 0x62, 0x73, ] => Some(&[0xe2, 0x99, 0xa3, ]),
        //"Tau" => "Τ"
        &[0x54, 0x61, 0x75, ] => Some(&[0xce, 0xa4, ]),
        //"Ecirc" => "Ê"
        &[0x45, 0x63, 0x69, 0x72, 0x63, ] => Some(&[0xc3, 0x8a, ]),
        //"loz" => "◊"
        &[0x6c, 0x6f, 0x7a, ] => Some(&[0xe2, 0x97, 0x8a, ]),
        //"nlt" => "≮"
        &[0x6e, 0x6c, 0x74, ] => Some(&[0xe2, 0x89, 0xae, ]),
        //"angmsd" => "∡"
        &[0x61, 0x6e, 0x67, 0x6d, 0x73, 0x64, ] => Some(&[0xe2, 0x88, 0xa1, ]),
        //"rlm" => "‏"
        &[0x72, 0x6c, 0x6d, ] => Some(&[0xe2, 0x80, 0x8f, ]),
        //"Nu" => "Ν"
        &[0x4e, 0x75, ] => Some(&[0xce, 0x9d, ]),
        //"conint" => "∮"
        &[0x63, 0x6f, 0x6e, 0x69, 0x6e, 0x74, ] => Some(&[0xe2, 0x88, 0xae, ]),
        //"Egrave" => "È"
        &[0x45, 0x67, 0x72, 0x61, 0x76, 0x65, ] => Some(&[0xc3, 0x88, ]),
        //"szlig" => "ß"
        &[0x73, 0x7a, 0x6c, 0x69, 0x67, ] => Some(&[0xc3, 0x9f, ]),
        //"cup" => "∪"
        &[0x63, 0x75, 0x70, ] => Some(&[0xe2, 0x88, 0xaa, ]),
        //"piv" => "ϖ"
        &[0x70, 0x69, 0x76, ] => Some(&[0xcf, 0x96, ]),
        //"Zeta" => "Ζ"
        &[0x5a, 0x65, 0x74, 0x61, ] => Some(&[0xce, 0x96, ]),
        //"gt" => ">"
        &[0x67, 0x74, ] => Some(&[0x3e, ]),
        //"darr" => "↓"
        &[0x64, 0x61, 0x72, 0x72, ] => Some(&[0xe2, 0x86, 0x93, ]),
        //"frac14" => "¼"
        &[0x66, 0x72, 0x61, 0x63, 0x31, 0x34, ] => Some(&[0xc2, 0xbc, ]),
        //"nges" => "≱"
        &[0x6e, 0x67, 0x65, 0x73, ] => Some(&[0xe2, 0x89, 0xb1, ]),
        //"frasl" => "⁄"
        &[0x66, 0x72, 0x61, 0x73, 0x6c, ] => Some(&[0xe2, 0x81, 0x84, ]),
        //"minus" => "−"
        &[0x6d, 0x69, 0x6e, 0x75, 0x73, ] => Some(&[0xe2, 0x88, 0x92, ]),
        //"uarr" => "↑"
        &[0x75, 0x61, 0x72, 0x72, ] => Some(&[0xe2, 0x86, 0x91, ]),
        //"zeta" => "ζ"
        &[0x7a, 0x65, 0x74, 0x61, ] => Some(&[0xce, 0xb6, ]),
        //"Iota" => "Ι"
        &[0x49, 0x6f, 0x74, 0x61, ] => Some(&[0xce, 0x99, ]),
        //"atilde" => "ã"
        &[0x61, 0x74, 0x69, 0x6c, 0x64, 0x65, ] => Some(&[0xc3, 0xa3, ]),
        //"agrave" => "à"
        &[0x61, 0x67, 0x72, 0x61, 0x76, 0x65, ] => Some(&[0xc3, 0xa0, ]),
        //"Aacute" => "Á"
        &[0x41, 0x61, 0x63, 0x75, 0x74, 0x65, ] => Some(&[0xc3, 0x81, ]),
        //"ensp" => " "
        &[0x65, 0x6e, 0x73, 0x70, ] => Some(&[0xe2, 0x80, 0x82, ]),
        //"mu" => "μ"
        &[0x6d, 0x75, ] => Some(&[0xce, 0xbc, ]),
        //"ocirc" => "ô"
        &[0x6f, 0x63, 0x69, 0x72, 0x63, ] => Some(&[0xc3, 0xb4, ]),
        //"deg" => "°"
        &[0x64, 0x65, 0x67, ] => Some(&[0xc2, 0xb0, ]),
        //"alefsym" => "ℵ"
        &[0x61, 0x6c, 0x65, 0x66, 0x73, 0x79, 0x6d, ] => Some(&[0xe2, 0x84, 0xb5, ]),
        //"prime" => "′"
        &[0x70, 0x72, 0x69, 0x6d, 0x65, ] => Some(&[0xe2, 0x80, 0xb2, ]),
        //"Gamma" => "Γ"
        &[0x47, 0x61, 0x6d, 0x6d, 0x61, ] => Some(&[0xce, 0x93, ]),
        //"Sigma" => "Σ"
        &[0x53, 0x69, 0x67, 0x6d, 0x61, ] => Some(&[0xce, 0xa3, ]),
        //"sdot" => "⋅"
        &[0x73, 0x64, 0x6f, 0x74, ] => Some(&[0xe2, 0x8b, 0x85, ]),
        //"par" => "∥"
        &[0x70, 0x61, 0x72, ] => Some(&[0xe2, 0x88, 0xa5, ]),
        //"comet" => "☄"
        &[0x63, 0x6f, 0x6d, 0x65, 0x74, ] => Some(&[0xe2, 0x98, 0x84, ]),
        //"and" => "∧"
        &[0x61, 0x6e, 0x64, ] => Some(&[0xe2, 0x88, 0xa7, ]),
        //"ndash" => "–"
        &[0x6e, 0x64, 0x61, 0x73, 0x68, ] => Some(&[0xe2, 0x80, 0x93, ]),
        //"oelig" => "œ"
        &[0x6f, 0x65, 0x6c, 0x69, 0x67, ] => Some(&[0xc5, 0x93, ]),
        //"compfn" => "∘"
        &[0x63, 0x6f, 0x6d, 0x70, 0x66, 0x6e, ] => Some(&[0xe2, 0x88, 0x98, ]),
        //"lAarr" => "⇚"
        &[0x6c, 0x41, 0x61, 0x72, 0x72, ] => Some(&[0xe2, 0x87, 0x9a, ]),
        //"Euml" => "Ë"
        &[0x45, 0x75, 0x6d, 0x6c, ] => Some(&[0xc3, 0x8b, ]),
        //"lsaquo" => "‹"
        &[0x6c, 0x73, 0x61, 0x71, 0x75, 0x6f, ] => Some(&[0xe2, 0x80, 0xb9, ]),
        //"thinsp" => " "
        &[0x74, 0x68, 0x69, 0x6e, 0x73, 0x70, ] => Some(&[0xe2, 0x80, 0x89, ]),
        //"omicron" => "ο"
        &[0x6f, 0x6d, 0x69, 0x63, 0x72, 0x6f, 0x6e, ] => Some(&[0xce, 0xbf, ]),
        //"thunderstorm" => "☈"
        &[0x74, 0x68, 0x75, 0x6e, 0x64, 0x65, 0x72, 0x73, 0x74, 0x6f, 0x72, 0x6d, ] => Some(&[0xe2, 0x98, 0x88, ]),
        //"cloud" => "☁"
        &[0x63, 0x6c, 0x6f, 0x75, 0x64, ] => Some(&[0xe2, 0x98, 0x81, ]),
        //"mnplus" => "∓"
        &[0x6d, 0x6e, 0x70, 0x6c, 0x75, 0x73, ] => Some(&[0xe2, 0x88, 0x93, ]),
        //"nsup" => "⊅"
        &[0x6e, 0x73, 0x75, 0x70, ] => Some(&[0xe2, 0x8a, 0x85, ]),
        //"mdash" => "—"
        &[0x6d, 0x64, 0x61, 0x73, 0x68, ] => Some(&[0xe2, 0x80, 0x94, ]),
        //"twixt" => "≬"
        &[0x74, 0x77, 0x69, 0x78, 0x74, ] => Some(&[0xe2, 0x89, 0xac, ]),
        //"angsph" => "∢"
        &[0x61, 0x6e, 0x67, 0x73, 0x70, 0x68, ] => Some(&[0xe2, 0x88, 0xa2, ]),
        //"Delta" => "Δ"
        &[0x44, 0x65, 0x6c, 0x74, 0x61, ] => Some(&[0xce, 0x94, ]),
        //"lambda" => "λ"
        &[0x6c, 0x61, 0x6d, 0x62, 0x64, 0x61, ] => Some(&[0xce, 0xbb, ]),
        //"Eta" => "Η"
        &[0x45, 0x74, 0x61, ] => Some(&[0xce, 0x97, ]),
        //"Theta" => "Θ"
        &[0x54, 0x68, 0x65, 0x74, 0x61, ] => Some(&[0xce, 0x98, ]),
        //"crarr" => "↵"
        &[0x63, 0x72, 0x61, 0x72, 0x72, ] => Some(&[0xe2, 0x86, 0xb5, ]),
        //"Chi" => "Χ"
        &[0x43, 0x68, 0x69, ] => Some(&[0xce, 0xa7, ]),
        //"sup3" => "³"
        &[0x73, 0x75, 0x70, 0x33, ] => Some(&[0xc2, 0xb3, ]),
        //"snowflake" => "❅"
        &[0x73, 0x6e, 0x6f, 0x77, 0x66, 0x6c, 0x61, 0x6b, 0x65, ] => Some(&[0xe2, 0x9d, 0x85, ]),
        //"plusdo" => "∔"
        &[0x70, 0x6c, 0x75, 0x73, 0x64, 0x6f, ] => Some(&[0xe2, 0x88, 0x94, ]),
        //"supe" => "⊇"
        &[0x73, 0x75, 0x70, 0x65, ] => Some(&[0xe2, 0x8a, 0x87, ]),
        //"Lt" => "≪"
        &[0x4c, 0x74, ] => Some(&[0xe2, 0x89, 0xaa, ]),
        //"prop" => "∝"
        &[0x70, 0x72, 0x6f, 0x70, ] => Some(&[0xe2, 0x88, 0x9d, ]),
        //"frac34" => "¾"
        &[0x66, 0x72, 0x61, 0x63, 0x33, 0x34, ] => Some(&[0xc2, 0xbe, ]),
        //"sup2" => "²"
        &[0x73, 0x75, 0x70, 0x32, ] => Some(&[0xc2, 0xb2, ]),
        //"reg" => "®"
        &[0x72, 0x65, 0x67, ] => Some(&[0xc2, 0xae, ]),
        //"isin" => "∈"
        &[0x69, 0x73, 0x69, 0x6e, ] => Some(&[0xe2, 0x88, 0x88, ]),
        //"sube" => "⊆"
        &[0x73, 0x75, 0x62, 0x65, ] => Some(&[0xe2, 0x8a, 0x86, ]),
        //"rAarr" => "⇛"
        &[0x72, 0x41, 0x61, 0x72, 0x72, ] => Some(&[0xe2, 0x87, 0x9b, ]),
        //"gl" => "≷"
        &[0x67, 0x6c, ] => Some(&[0xe2, 0x89, 0xb7, ]),
        //"sime" => "≃"
        &[0x73, 0x69, 0x6d, 0x65, ] => Some(&[0xe2, 0x89, 0x83, ]),
        //"nsub" => "⊄"
        &[0x6e, 0x73, 0x75, 0x62, ] => Some(&[0xe2, 0x8a, 0x84, ]),
        //"hArr" => "⇔"
        &[0x68, 0x41, 0x72, 0x72, ] => Some(&[0xe2, 0x87, 0x94, ]),
        //"icirc" => "î"
        &[0x69, 0x63, 0x69, 0x72, 0x63, ] => Some(&[0xc3, 0xae, ]),
        //"ne" => "≠"
        &[0x6e, 0x65, ] => Some(&[0xe2, 0x89, 0xa0, ]),
        //"ucirc" => "û"
        &[0x75, 0x63, 0x69, 0x72, 0x63, ] => Some(&[0xc3, 0xbb, ]),
        //"coprod" => "∐"
        &[0x63, 0x6f, 0x70, 0x72, 0x6f, 0x64, ] => Some(&[0xe2, 0x88, 0x90, ]),
        //"oacute" => "ó"
        &[0x6f, 0x61, 0x63, 0x75, 0x74, 0x65, ] => Some(&[0xc3, 0xb3, ]),
        //"cent" => "¢"
        &[0x63, 0x65, 0x6e, 0x74, ] => Some(&[0xc2, 0xa2, ]),
        //"nsc" => "⊁"
        &[0x6e, 0x73, 0x63, ] => Some(&[0xe2, 0x8a, 0x81, ]),
        //"cupre" => "≼"
        &[0x63, 0x75, 0x70, 0x72, 0x65, ] => Some(&[0xe2, 0x89, 0xbc, ]),
        //"lArr" => "⇐"
        &[0x6c, 0x41, 0x72, 0x72, ] => Some(&[0xe2, 0x87, 0x90, ]),
        //"pi" => "π"
        &[0x70, 0x69, ] => Some(&[0xcf, 0x80, ]),
        //"plusmn" => "±"
        &[0x70, 0x6c, 0x75, 0x73, 0x6d, 0x6e, ] => Some(&[0xc2, 0xb1, ]),
        //"Phi" => "Φ"
        &[0x50, 0x68, 0x69, ] => Some(&[0xce, 0xa6, ]),
        //"infin" => "∞"
        &[0x69, 0x6e, 0x66, 0x69, 0x6e, ] => Some(&[0xe2, 0x88, 0x9e, ]),
        //"divide" => "÷"
        &[0x64, 0x69, 0x76, 0x69, 0x64, 0x65, ] => Some(&[0xc3, 0xb7, ]),
        //"tau" => "τ"
        &[0x74, 0x61, 0x75, ] => Some(&[0xcf, 0x84, ]),
        //"frac12" => "½"
        &[0x66, 0x72, 0x61, 0x63, 0x31, 0x32, ] => Some(&[0xc2, 0xbd, ]),
        //"equiv" => "≡"
        &[0x65, 0x71, 0x75, 0x69, 0x76, ] => Some(&[0xe2, 0x89, 0xa1, ]),
        //"bump" => "≎"
        &[0x62, 0x75, 0x6d, 0x70, ] => Some(&[0xe2, 0x89, 0x8e, ]),
        //"THORN" => "Þ"
        &[0x54, 0x48, 0x4f, 0x52, 0x4e, ] => Some(&[0xc3, 0x9e, ]),
        //"oline" => "‾"
        &[0x6f, 0x6c, 0x69, 0x6e, 0x65, ] => Some(&[0xe2, 0x80, 0xbe, ]),
        //"Mu" => "Μ"
        &[0x4d, 0x75, ] => Some(&[0xce, 0x9c, ]),
        //"sub" => "⊂"
        &[0x73, 0x75, 0x62, ] => Some(&[0xe2, 0x8a, 0x82, ]),
        //"shy" => "­"
        &[0x73, 0x68, 0x79, ] => Some(&[0xc2, 0xad, ]),
        //"nsim" => "≁"
        &[0x6e, 0x73, 0x69, 0x6d, ] => Some(&[0xe2, 0x89, 0x81, ]),
        //"thetasym" => "ϑ"
        &[0x74, 0x68, 0x65, 0x74, 0x61, 0x73, 0x79, 0x6d, ] => Some(&[0xcf, 0x91, ]),
        //"Omega" => "Ω"
        &[0x4f, 0x6d, 0x65, 0x67, 0x61, ] => Some(&[0xce, 0xa9, ]),
        //"Oslash" => "Ø"
        &[0x4f, 0x73, 0x6c, 0x61, 0x73, 0x68, ] => Some(&[0xc3, 0x98, ]),
        //"ang90" => "∟"
        &[0x61, 0x6e, 0x67, 0x39, 0x30, ] => Some(&[0xe2, 0x88, 0x9f, ]),
        //"iexcl" => "¡"
        &[0x69, 0x65, 0x78, 0x63, 0x6c, ] => Some(&[0xc2, 0xa1, ]),
        //"rArr" => "⇒"
        &[0x72, 0x41, 0x72, 0x72, ] => Some(&[0xe2, 0x87, 0x92, ]),
        //"cedil" => "¸"
        &[0x63, 0x65, 0x64, 0x69, 0x6c, ] => Some(&[0xc2, 0xb8, ]),
        //"uacute" => "ú"
        &[0x75, 0x61, 0x63, 0x75, 0x74, 0x65, ] => Some(&[0xc3, 0xba, ]),
        //"sup" => "⊃"
        &[0x73, 0x75, 0x70, ] => Some(&[0xe2, 0x8a, 0x83, ]),
        //"lE" => "≦"
        &[0x6c, 0x45, ] => Some(&[0xe2, 0x89, 0xa6, ]),
        //"sum" => "∑"
        &[0x73, 0x75, 0x6d, ] => Some(&[0xe2, 0x88, 0x91, ]),
        //"ntilde" => "ñ"
        &[0x6e, 0x74, 0x69, 0x6c, 0x64, 0x65, ] => Some(&[0xc3, 0xb1, ]),
        //"lceil" => "⌈"
        &[0x6c, 0x63, 0x65, 0x69, 0x6c, ] => Some(&[0xe2, 0x8c, 0x88, ]),
        //"bcong" => "≌"
        &[0x62, 0x63, 0x6f, 0x6e, 0x67, ] => Some(&[0xe2, 0x89, 0x8c, ]),
        //"mid" => "∣"
        &[0x6d, 0x69, 0x64, ] => Some(&[0xe2, 0x88, 0xa3, ]),
        //"dArr" => "⇓"
        &[0x64, 0x41, 0x72, 0x72, ] => Some(&[0xe2, 0x87, 0x93, ]),
        //"sigma" => "σ"
        &[0x73, 0x69, 0x67, 0x6d, 0x61, ] => Some(&[0xcf, 0x83, ]),
        //"nsime" => "≄"
        &[0x6e, 0x73, 0x69, 0x6d, 0x65, ] => Some(&[0xe2, 0x89, 0x84, ]),
        //"Xi" => "Ξ"
        &[0x58, 0x69, ] => Some(&[0xce, 0x9e, ]),
        //"sc" => "≻"
        &[0x73, 0x63, ] => Some(&[0xe2, 0x89, 0xbb, ]),
        //"Lambda" => "Λ"
        &[0x4c, 0x61, 0x6d, 0x62, 0x64, 0x61, ] => Some(&[0xce, 0x9b, ]),
        //"oslash" => "ø"
        &[0x6f, 0x73, 0x6c, 0x61, 0x73, 0x68, ] => Some(&[0xc3, 0xb8, ]),
        //"forall" => "∀"
        &[0x66, 0x6f, 0x72, 0x61, 0x6c, 0x6c, ] => Some(&[0xe2, 0x88, 0x80, ]),
        //"umbrella" => "☂"
        &[0x75, 0x6d, 0x62, 0x72, 0x65, 0x6c, 0x6c, 0x61, ] => Some(&[0xe2, 0x98, 0x82, ]),
        //"uArr" => "⇑"
        &[0x75, 0x41, 0x72, 0x72, ] => Some(&[0xe2, 0x87, 0x91, ]),
        //"diams" => "♦"
        &[0x64, 0x69, 0x61, 0x6d, 0x73, ] => Some(&[0xe2, 0x99, 0xa6, ]),
        //"iquest" => "¿"
        &[0x69, 0x71, 0x75, 0x65, 0x73, 0x74, ] => Some(&[0xc2, 0xbf, ]),
        //"eta" => "η"
        &[0x65, 0x74, 0x61, ] => Some(&[0xce, 0xb7, ]),
        //"gamma" => "γ"
        &[0x67, 0x61, 0x6d, 0x6d, 0x61, ] => Some(&[0xce, 0xb3, ]),
        //"iuml" => "ï"
        &[0x69, 0x75, 0x6d, 0x6c, ] => Some(&[0xc3, 0xaf, ]),
        //"middot" => "·"
        &[0x6d, 0x69, 0x64, 0x64, 0x6f, 0x74, ] => Some(&[0xc2, 0xb7, ]),
        //"gE" => "≧"
        &[0x67, 0x45, ] => Some(&[0xe2, 0x89, 0xa7, ]),
        //"dagger" => "†"
        &[0x64, 0x61, 0x67, 0x67, 0x65, 0x72, ] => Some(&[0xe2, 0x80, 0xa0, ]),
        //"weierp" => "℘"
        &[0x77, 0x65, 0x69, 0x65, 0x72, 0x70, ] => Some(&[0xe2, 0x84, 0x98, ]),
        //"ouml" => "ö"
        &[0x6f, 0x75, 0x6d, 0x6c, ] => Some(&[0xc3, 0xb6, ]),
        //"perp" => "⊥"
        &[0x70, 0x65, 0x72, 0x70, ] => Some(&[0xe2, 0x8a, 0xa5, ]),
        //"curren" => "¤"
        &[0x63, 0x75, 0x72, 0x72, 0x65, 0x6e, ] => Some(&[0xc2, 0xa4, ]),
        //"amp" => "&"
        &[0x61, 0x6d, 0x70, ] => Some(&[0x26, ]),
        //"iota" => "ι"
        &[0x69, 0x6f, 0x74, 0x61, ] => Some(&[0xce, 0xb9, ]),
        //"quot" => """
        &[0x71, 0x75, 0x6f, 0x74, ] => Some(&[0x22, ]),
        //"ang" => "∠"
        &[0x61, 0x6e, 0x67, ] => Some(&[0xe2, 0x88, 0xa0, ]),
        //"Iuml" => "Ï"
        &[0x49, 0x75, 0x6d, 0x6c, ] => Some(&[0xc3, 0x8f, ]),
        //"spades" => "♠"
        &[0x73, 0x70, 0x61, 0x64, 0x65, 0x73, ] => Some(&[0xe2, 0x99, 0xa0, ]),
        //"ge" => "≥"
        &[0x67, 0x65, ] => Some(&[0xe2, 0x89, 0xa5, ]),
        //"image" => "ℑ"
        &[0x69, 0x6d, 0x61, 0x67, 0x65, ] => Some(&[0xe2, 0x84, 0x91, ]),
        //"psi" => "ψ"
        &[0x70, 0x73, 0x69, ] => Some(&[0xcf, 0x88, ]),
        //"Eacute" => "É"
        &[0x45, 0x61, 0x63, 0x75, 0x74, 0x65, ] => Some(&[0xc3, 0x89, ]),
        //"uuml" => "ü"
        &[0x75, 0x75, 0x6d, 0x6c, ] => Some(&[0xc3, 0xbc, ]),
        //"radic" => "√"
        &[0x72, 0x61, 0x64, 0x69, 0x63, ] => Some(&[0xe2, 0x88, 0x9a, ]),
        //"ni" => "∋"
        &[0x6e, 0x69, ] => Some(&[0xe2, 0x88, 0x8b, ]),
        //"bull" => "•"
        &[0x62, 0x75, 0x6c, 0x6c, ] => Some(&[0xe2, 0x80, 0xa2, ]),
        //"times" => "×"
        &[0x74, 0x69, 0x6d, 0x65, 0x73, ] => Some(&[0xc3, 0x97, ]),
        //"AElig" => "Æ"
        &[0x41, 0x45, 0x6c, 0x69, 0x67, ] => Some(&[0xc3, 0x86, ]),
        //"ordm" => "º"
        &[0x6f, 0x72, 0x64, 0x6d, ] => Some(&[0xc2, 0xba, ]),
        //"prsim" => "≾"
        &[0x70, 0x72, 0x73, 0x69, 0x6d, ] => Some(&[0xe2, 0x89, 0xbe, ]),
        //"bepsi" => "∍"
        &[0x62, 0x65, 0x70, 0x73, 0x69, ] => Some(&[0xe2, 0x88, 0x8d, ]),
        //"epsis" => "∊"
        &[0x65, 0x70, 0x73, 0x69, 0x73, ] => Some(&[0xe2, 0x88, 0x8a, ]),
        //"vArr" => "⇕"
        &[0x76, 0x41, 0x72, 0x72, ] => Some(&[0xe2, 0x87, 0x95, ]),
        //"ngt" => "≯"
        &[0x6e, 0x67, 0x74, ] => Some(&[0xe2, 0x89, 0xaf, ]),
        //"part" => "∂"
        &[0x70, 0x61, 0x72, 0x74, ] => Some(&[0xe2, 0x88, 0x82, ]),
        //"otimes" => "⊗"
        &[0x6f, 0x74, 0x69, 0x6d, 0x65, 0x73, ] => Some(&[0xe2, 0x8a, 0x97, ]),
        //"micro" => "µ"
        &[0x6d, 0x69, 0x63, 0x72, 0x6f, ] => Some(&[0xc2, 0xb5, ]),
        //"raquo" => "»"
        &[0x72, 0x61, 0x71, 0x75, 0x6f, ] => Some(&[0xc2, 0xbb, ]),
        //"Ocirc" => "Ô"
        &[0x4f, 0x63, 0x69, 0x72, 0x63, ] => Some(&[0xc3, 0x94, ]),
        //"macr" => "¯"
        &[0x6d, 0x61, 0x63, 0x72, ] => Some(&[0xc2, 0xaf, ]),
        //"Upsilon" => "Υ"
        &[0x55, 0x70, 0x73, 0x69, 0x6c, 0x6f, 0x6e, ] => Some(&[0xce, 0xa5, ]),
        //"Auml" => "Ä"
        &[0x41, 0x75, 0x6d, 0x6c, ] => Some(&[0xc3, 0x84, ]),
        //"sup1" => "¹"
        &[0x73, 0x75, 0x70, 0x31, ] => Some(&[0xc2, 0xb9, ]),
        //"iacute" => "í"
        &[0x69, 0x61, 0x63, 0x75, 0x74, 0x65, ] => Some(&[0xc3, 0xad, ]),
        //"ETH" => "Ð"
        &[0x45, 0x54, 0x48, ] => Some(&[0xc3, 0x90, ]),
        //"Icirc" => "Î"
        &[0x49, 0x63, 0x69, 0x72, 0x63, ] => Some(&[0xc3, 0x8e, ]),
        //"or" => "∨"
        &[0x6f, 0x72, ] => Some(&[0xe2, 0x88, 0xa8, ]),
        //"sigmaf" => "ς"
        &[0x73, 0x69, 0x67, 0x6d, 0x61, 0x66, ] => Some(&[0xcf, 0x82, ]),
        //"bumpe" => "≏"
        &[0x62, 0x75, 0x6d, 0x70, 0x65, ] => Some(&[0xe2, 0x89, 0x8f, ]),
        //"phi" => "φ"
        &[0x70, 0x68, 0x69, ] => Some(&[0xcf, 0x86, ]),
        //"pr" => "≺"
        &[0x70, 0x72, ] => Some(&[0xe2, 0x89, 0xba, ]),
        //"Ucirc" => "Û"
        &[0x55, 0x63, 0x69, 0x72, 0x63, ] => Some(&[0xc3, 0x9b, ]),
        //"beta" => "β"
        &[0x62, 0x65, 0x74, 0x61, ] => Some(&[0xce, 0xb2, ]),
        //"aring" => "å"
        &[0x61, 0x72, 0x69, 0x6e, 0x67, ] => Some(&[0xc3, 0xa5, ]),
        //"lfloor" => "⌊"
        &[0x6c, 0x66, 0x6c, 0x6f, 0x6f, 0x72, ] => Some(&[0xe2, 0x8c, 0x8a, ]),
        //"Epsilon" => "Ε"
        &[0x45, 0x70, 0x73, 0x69, 0x6c, 0x6f, 0x6e, ] => Some(&[0xce, 0x95, ]),
        //"upsih" => "ϒ"
        &[0x75, 0x70, 0x73, 0x69, 0x68, ] => Some(&[0xcf, 0x92, ]),
        //"thorn" => "þ"
        &[0x74, 0x68, 0x6f, 0x72, 0x6e, ] => Some(&[0xc3, 0xbe, ]),
        //"Oacute" => "Ó"
        &[0x4f, 0x61, 0x63, 0x75, 0x74, 0x65, ] => Some(&[0xc3, 0x93, ]),
        //"rarrw" => "⇝"
        &[0x72, 0x61, 0x72, 0x72, 0x77, ] => Some(&[0xe2, 0x87, 0x9d, ]),
        //"larr" => "←"
        &[0x6c, 0x61, 0x72, 0x72, ] => Some(&[0xe2, 0x86, 0x90, ]),
        //"aelig" => "æ"
        &[0x61, 0x65, 0x6c, 0x69, 0x67, ] => Some(&[0xc3, 0xa6, ]),
        //"gnE" => "≩"
        &[0x67, 0x6e, 0x45, ] => Some(&[0xe2, 0x89, 0xa9, ]),
        //"brvbar" => "¦"
        &[0x62, 0x72, 0x76, 0x62, 0x61, 0x72, ] => Some(&[0xc2, 0xa6, ]),
        //"asympeq" => "≍"
        &[0x61, 0x73, 0x79, 0x6d, 0x70, 0x65, 0x71, ] => Some(&[0xe2, 0x89, 0x8d, ]),
        //"int" => "∫"
        &[0x69, 0x6e, 0x74, ] => Some(&[0xe2, 0x88, 0xab, ]),
        //"ccedil" => "ç"
        &[0x63, 0x63, 0x65, 0x64, 0x69, 0x6c, ] => Some(&[0xc3, 0xa7, ]),
        //"npar" => "∦"
        &[0x6e, 0x70, 0x61, 0x72, ] => Some(&[0xe2, 0x88, 0xa6, ]),
        //"lsquo" => "‘"
        &[0x6c, 0x73, 0x71, 0x75, 0x6f, ] => Some(&[0xe2, 0x80, 0x98, ]),
        //"harr" => "↔"
        &[0x68, 0x61, 0x72, 0x72, ] => Some(&[0xe2, 0x86, 0x94, ]),
        //"Rho" => "Ρ"
        &[0x52, 0x68, 0x6f, ] => Some(&[0xce, 0xa1, ]),
        //"pound" => "£"
        &[0x70, 0x6f, 0x75, 0x6e, 0x64, ] => Some(&[0xc2, 0xa3, ]),
        //"apos" => "'"
        &[0x61, 0x70, 0x6f, 0x73, ] => Some(&[0x27, ]),
        //"real" => "ℜ"
        &[0x72, 0x65, 0x61, 0x6c, ] => Some(&[0xe2, 0x84, 0x9c, ]),
        //"hellip" => "…"
        &[0x68, 0x65, 0x6c, 0x6c, 0x69, 0x70, ] => Some(&[0xe2, 0x80, 0xa6, ]),
        //"Ouml" => "Ö"
        &[0x4f, 0x75, 0x6d, 0x6c, ] => Some(&[0xc3, 0x96, ]),
        //"euml" => "ë"
        &[0x65, 0x75, 0x6d, 0x6c, ] => Some(&[0xc3, 0xab, ]),
        //"uml" => "¨"
        &[0x75, 0x6d, 0x6c, ] => Some(&[0xc2, 0xa8, ]),
        //"Kappa" => "Κ"
        &[0x4b, 0x61, 0x70, 0x70, 0x61, ] => Some(&[0xce, 0x9a, ]),
        //"rceil" => "⌉"
        &[0x72, 0x63, 0x65, 0x69, 0x6c, ] => Some(&[0xe2, 0x8c, 0x89, ]),
        //"notni" => "∌"
        &[0x6e, 0x6f, 0x74, 0x6e, 0x69, ] => Some(&[0xe2, 0x88, 0x8c, ]),
        //"ugrave" => "ù"
        &[0x75, 0x67, 0x72, 0x61, 0x76, 0x65, ] => Some(&[0xc3, 0xb9, ]),
        //"acirc" => "â"
        &[0x61, 0x63, 0x69, 0x72, 0x63, ] => Some(&[0xc3, 0xa2, ]),
        //"laquo" => "«"
        &[0x6c, 0x61, 0x71, 0x75, 0x6f, ] => Some(&[0xc2, 0xab, ]),
        //"ncong" => "≇"
        &[0x6e, 0x63, 0x6f, 0x6e, 0x67, ] => Some(&[0xe2, 0x89, 0x87, ]),
        //"para" => "¶"
        &[0x70, 0x61, 0x72, 0x61, ] => Some(&[0xc2, 0xb6, ]),
        //"asymp" => "≈"
        &[0x61, 0x73, 0x79, 0x6d, 0x70, ] => Some(&[0xe2, 0x89, 0x88, ]),
        //"Agrave" => "À"
        &[0x41, 0x67, 0x72, 0x61, 0x76, 0x65, ] => Some(&[0xc3, 0x80, ]),
        //"Pi" => "Π"
        &[0x50, 0x69, ] => Some(&[0xce, 0xa0, ]),
        //"aacute" => "á"
        &[0x61, 0x61, 0x63, 0x75, 0x74, 0x65, ] => Some(&[0xc3, 0xa1, ]),
        //"gsim" => "≳"
        &[0x67, 0x73, 0x69, 0x6d, ] => Some(&[0xe2, 0x89, 0xb3, ]),
        //"rfloor" => "⌋"
        &[0x72, 0x66, 0x6c, 0x6f, 0x6f, 0x72, ] => Some(&[0xe2, 0x8c, 0x8b, ]),
        //"rarr" => "→"
        &[0x72, 0x61, 0x72, 0x72, ] => Some(&[0xe2, 0x86, 0x92, ]),
        //"ecirc" => "ê"
        &[0x65, 0x63, 0x69, 0x72, 0x63, ] => Some(&[0xc3, 0xaa, ]),
        //"delta" => "δ"
        &[0x64, 0x65, 0x6c, 0x74, 0x61, ] => Some(&[0xce, 0xb4, ]),
        //"Ograve" => "Ò"
        &[0x4f, 0x67, 0x72, 0x61, 0x76, 0x65, ] => Some(&[0xc3, 0x92, ]),
        //"there4" => "∴"
        &[0x74, 0x68, 0x65, 0x72, 0x65, 0x34, ] => Some(&[0xe2, 0x88, 0xb4, ]),
        //"Prime" => "″"
        &[0x50, 0x72, 0x69, 0x6d, 0x65, ] => Some(&[0xe2, 0x80, 0xb3, ]),
        //"sect" => "§"
        &[0x73, 0x65, 0x63, 0x74, ] => Some(&[0xc2, 0xa7, ]),
        //"empty" => "∅"
        &[0x65, 0x6d, 0x70, 0x74, 0x79, ] => Some(&[0xe2, 0x88, 0x85, ]),
        //"Omicron" => "Ο"
        &[0x4f, 0x6d, 0x69, 0x63, 0x72, 0x6f, 0x6e, ] => Some(&[0xce, 0x9f, ]),
        //"tilde" => "˜"
        &[0x74, 0x69, 0x6c, 0x64, 0x65, ] => Some(&[0xcb, 0x9c, ]),
        //"fnof" => "ƒ"
        &[0x66, 0x6e, 0x6f, 0x66, ] => Some(&[0xc6, 0x92, ]),
        //"eth" => "ð"
        &[0x65, 0x74, 0x68, ] => Some(&[0xc3, 0xb0, ]),
        //"ordf" => "ª"
        &[0x6f, 0x72, 0x64, 0x66, ] => Some(&[0xc2, 0xaa, ]),
        //"zwj" => "‍"
        &[0x7a, 0x77, 0x6a, ] => Some(&[0xe2, 0x80, 0x8d, ]),
        //"nmid" => "∤"
        &[0x6e, 0x6d, 0x69, 0x64, ] => Some(&[0xe2, 0x88, 0xa4, ]),
        //"rho" => "ρ"
        &[0x72, 0x68, 0x6f, ] => Some(&[0xcf, 0x81, ]),
        //"auml" => "ä"
        &[0x61, 0x75, 0x6d, 0x6c, ] => Some(&[0xc3, 0xa4, ]),
        //"lnE" => "≨"
        &[0x6c, 0x6e, 0x45, ] => Some(&[0xe2, 0x89, 0xa8, ]),
        //"zwnj" => "‌"
        &[0x7a, 0x77, 0x6e, 0x6a, ] => Some(&[0xe2, 0x80, 0x8c, ]),
        //"Uacute" => "Ú"
        &[0x55, 0x61, 0x63, 0x75, 0x74, 0x65, ] => Some(&[0xc3, 0x9a, ]),
        //"yuml" => "ÿ"
        &[0x79, 0x75, 0x6d, 0x6c, ] => Some(&[0xc3, 0xbf, ]),
        //"Aring" => "Å"
        &[0x41, 0x72, 0x69, 0x6e, 0x67, ] => Some(&[0xc3, 0x85, ]),
        //"lsim" => "≲"
        &[0x6c, 0x73, 0x69, 0x6d, ] => Some(&[0xe2, 0x89, 0xb2, ]),
        //"nap" => "≉"
        &[0x6e, 0x61, 0x70, ] => Some(&[0xe2, 0x89, 0x89, ]),
        //"Scaron" => "Š"
        &[0x53, 0x63, 0x61, 0x72, 0x6f, 0x6e, ] => Some(&[0xc5, 0xa0, ]),
        //"acute" => "´"
        &[0x61, 0x63, 0x75, 0x74, 0x65, ] => Some(&[0xc2, 0xb4, ]),
        //"yacute" => "ý"
        &[0x79, 0x61, 0x63, 0x75, 0x74, 0x65, ] => Some(&[0xc3, 0xbd, ]),
        //"lowast" => "∗"
        &[0x6c, 0x6f, 0x77, 0x61, 0x73, 0x74, ] => Some(&[0xe2, 0x88, 0x97, ]),
        //"egrave" => "è"
        &[0x65, 0x67, 0x72, 0x61, 0x76, 0x65, ] => Some(&[0xc3, 0xa8, ]),
        //"Acirc" => "Â"
        &[0x41, 0x63, 0x69, 0x72, 0x63, ] => Some(&[0xc3, 0x82, ]),
        //"euro" => "€"
        &[0x65, 0x75, 0x72, 0x6f, ] => Some(&[0xe2, 0x82, 0xac, ]),
        //"Gt" => "≫"
        &[0x47, 0x74, ] => Some(&[0xe2, 0x89, 0xab, ]),
        //"igrave" => "ì"
        &[0x69, 0x67, 0x72, 0x61, 0x76, 0x65, ] => Some(&[0xc3, 0xac, ]),
        //"cap" => "∩"
        &[0x63, 0x61, 0x70, ] => Some(&[0xe2, 0x88, 0xa9, ]),
        //"Otilde" => "Õ"
        &[0x4f, 0x74, 0x69, 0x6c, 0x64, 0x65, ] => Some(&[0xc3, 0x95, ]),
        //"scsim" => "≿"
        &[0x73, 0x63, 0x73, 0x69, 0x6d, ] => Some(&[0xe2, 0x89, 0xbf, ]),
        //"Igrave" => "Ì"
        &[0x49, 0x67, 0x72, 0x61, 0x76, 0x65, ] => Some(&[0xc3, 0x8c, ]),
        //"exist" => "∃"
        &[0x65, 0x78, 0x69, 0x73, 0x74, ] => Some(&[0xe2, 0x88, 0x83, ]),
        //"nu" => "ν"
        &[0x6e, 0x75, ] => Some(&[0xce, 0xbd, ]),
        //"omega" => "ω"
        &[0x6f, 0x6d, 0x65, 0x67, 0x61, ] => Some(&[0xcf, 0x89, ]),
        //"snowman" => "☃"
        &[0x73, 0x6e, 0x6f, 0x77, 0x6d, 0x61, 0x6e, ] => Some(&[0xe2, 0x98, 0x83, ]),
        //"cong" => "≅"
        &[0x63, 0x6f, 0x6e, 0x67, ] => Some(&[0xe2, 0x89, 0x85, ]),
        //"Ccedil" => "Ç"
        &[0x43, 0x63, 0x65, 0x64, 0x69, 0x6c, ] => Some(&[0xc3, 0x87, ]),
        //"rang" => "〉"
        &[0x72, 0x61, 0x6e, 0x67, ] => Some(&[0xe2, 0x8c, 0xaa, ]),
        //"setmn" => "∖"
        &[0x73, 0x65, 0x74, 0x6d, 0x6e, ] => Some(&[0xe2, 0x88, 0x96, ]),
        //"rdquo" => "”"
        &[0x72, 0x64, 0x71, 0x75, 0x6f, ] => Some(&[0xe2, 0x80, 0x9d, ]),
        //"yen" => "¥"
        &[0x79, 0x65, 0x6e, ] => Some(&[0xc2, 0xa5, ]),
        //"ograve" => "ò"
        &[0x6f, 0x67, 0x72, 0x61, 0x76, 0x65, ] => Some(&[0xc3, 0xb2, ]),
        //"Psi" => "Ψ"
        &[0x50, 0x73, 0x69, ] => Some(&[0xce, 0xa8, ]),
        //"lt" => "<"
        &[0x6c, 0x74, ] => Some(&[0x3c, ]),
        //"epsilon" => "ε"
        &[0x65, 0x70, 0x73, 0x69, 0x6c, 0x6f, 0x6e, ] => Some(&[0xce, 0xb5, ]),
        //"alpha" => "α"
        &[0x61, 0x6c, 0x70, 0x68, 0x61, ] => Some(&[0xce, 0xb1, ]),
        //"sim" => "∼"
        &[0x73, 0x69, 0x6d, ] => Some(&[0xe2, 0x88, 0xbc, ]),
        //"Uuml" => "Ü"
        &[0x55, 0x75, 0x6d, 0x6c, ] => Some(&[0xc3, 0x9c, ]),
        //"not" => "¬"
        &[0x6e, 0x6f, 0x74, ] => Some(&[0xc2, 0xac, ]),
        //"nbsp" => " "
        &[0x6e, 0x62, 0x73, 0x70, ] => Some(&[0xc2, 0xa0, ]),
        //"circ" => "ˆ"
        &[0x63, 0x69, 0x72, 0x63, ] => Some(&[0xcb, 0x86, ]),
        //"copy" => "©"
        &[0x63, 0x6f, 0x70, 0x79, ] => Some(&[0xc2, 0xa9, ]),
        //"chi" => "χ"
        &[0x63, 0x68, 0x69, ] => Some(&[0xcf, 0x87, ]),
        //"Beta" => "Β"
        &[0x42, 0x65, 0x74, 0x61, ] => Some(&[0xce, 0x92, ]),
        //"Alpha" => "Α"
        &[0x41, 0x6c, 0x70, 0x68, 0x61, ] => Some(&[0xce, 0x91, ]),
        //"Ntilde" => "Ñ"
        &[0x4e, 0x74, 0x69, 0x6c, 0x64, 0x65, ] => Some(&[0xc3, 0x91, ]),
        //"Dagger" => "‡"
        &[0x44, 0x61, 0x67, 0x67, 0x65, 0x72, ] => Some(&[0xe2, 0x80, 0xa1, ]),
        //"sbquo" => "‚"
        &[0x73, 0x62, 0x71, 0x75, 0x6f, ] => Some(&[0xe2, 0x80, 0x9a, ]),
        //"eacute" => "é"
        &[0x65, 0x61, 0x63, 0x75, 0x74, 0x65, ] => Some(&[0xc3, 0xa9, ]),
        //"nles" => "≰"
        &[0x6e, 0x6c, 0x65, 0x73, ] => Some(&[0xe2, 0x89, 0xb0, ]),
        //"comp" => "∁"
        &[0x63, 0x6f, 0x6d, 0x70, ] => Some(&[0xe2, 0x88, 0x81, ]),
        //"Yacute" => "Ý"
        &[0x59, 0x61, 0x63, 0x75, 0x74, 0x65, ] => Some(&[0xc3, 0x9d, ]),
        //"Iacute" => "Í"
        &[0x49, 0x61, 0x63, 0x75, 0x74, 0x65, ] => Some(&[0xc3, 0x8d, ]),
        //"otilde" => "õ"
        &[0x6f, 0x74, 0x69, 0x6c, 0x64, 0x65, ] => Some(&[0xc3, 0xb5, ]),

        _ => None,
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use pretty_assertions::assert_eq;

    #[test]
    fn test() {
        assert_eq!(decode("bsim".as_bytes()).unwrap(), "∽".as_bytes());
        assert_eq!(decode("sscue".as_bytes()).unwrap(), "≽".as_bytes());
        assert_eq!(decode("becaus".as_bytes()).unwrap(), "∵".as_bytes());
        assert_eq!(decode("nexist".as_bytes()).unwrap(), "∄".as_bytes());
        assert_eq!(decode("Atilde".as_bytes()).unwrap(), "Ã".as_bytes());
        assert_eq!(decode("emsp".as_bytes()).unwrap(), " ".as_bytes());
        assert_eq!(decode("nabla".as_bytes()).unwrap(), "∇".as_bytes());
        assert_eq!(decode("lang".as_bytes()).unwrap(), "〈".as_bytes());
        assert_eq!(decode("Ugrave".as_bytes()).unwrap(), "Ù".as_bytes());
        assert_eq!(decode("hearts".as_bytes()).unwrap(), "♥".as_bytes());
        assert_eq!(decode("oplus".as_bytes()).unwrap(), "⊕".as_bytes());
        assert_eq!(decode("le".as_bytes()).unwrap(), "≤".as_bytes());
        assert_eq!(decode("wreath".as_bytes()).unwrap(), "≀".as_bytes());
        assert_eq!(decode("kappa".as_bytes()).unwrap(), "κ".as_bytes());
        assert_eq!(decode("lrm".as_bytes()).unwrap(), "‎".as_bytes());
        assert_eq!(decode("OElig".as_bytes()).unwrap(), "Œ".as_bytes());
        assert_eq!(decode("prod".as_bytes()).unwrap(), "∏".as_bytes());
        assert_eq!(decode("npr".as_bytes()).unwrap(), "⊀".as_bytes());
        assert_eq!(decode("notin".as_bytes()).unwrap(), "∉".as_bytes());
        assert_eq!(decode("rsaquo".as_bytes()).unwrap(), "›".as_bytes());
        assert_eq!(decode("upsilon".as_bytes()).unwrap(), "υ".as_bytes());
        assert_eq!(decode("lg".as_bytes()).unwrap(), "≶".as_bytes());
        assert_eq!(decode("trade".as_bytes()).unwrap(), "™".as_bytes());
        assert_eq!(decode("ape".as_bytes()).unwrap(), "≊".as_bytes());
        assert_eq!(decode("bdquo".as_bytes()).unwrap(), "„".as_bytes());
        assert_eq!(decode("theta".as_bytes()).unwrap(), "θ".as_bytes());
        assert_eq!(decode("ldquo".as_bytes()).unwrap(), "“".as_bytes());
        assert_eq!(decode("Yuml".as_bytes()).unwrap(), "Ÿ".as_bytes());
        assert_eq!(decode("scaron".as_bytes()).unwrap(), "š".as_bytes());
        assert_eq!(decode("permil".as_bytes()).unwrap(), "‰".as_bytes());
        assert_eq!(decode("xi".as_bytes()).unwrap(), "ξ".as_bytes());
        assert_eq!(decode("rsquo".as_bytes()).unwrap(), "’".as_bytes());
        assert_eq!(decode("clubs".as_bytes()).unwrap(), "♣".as_bytes());
        assert_eq!(decode("Tau".as_bytes()).unwrap(), "Τ".as_bytes());
        assert_eq!(decode("Ecirc".as_bytes()).unwrap(), "Ê".as_bytes());
        assert_eq!(decode("loz".as_bytes()).unwrap(), "◊".as_bytes());
        assert_eq!(decode("nlt".as_bytes()).unwrap(), "≮".as_bytes());
        assert_eq!(decode("angmsd".as_bytes()).unwrap(), "∡".as_bytes());
        assert_eq!(decode("rlm".as_bytes()).unwrap(), "‏".as_bytes());
        assert_eq!(decode("Nu".as_bytes()).unwrap(), "Ν".as_bytes());
        assert_eq!(decode("conint".as_bytes()).unwrap(), "∮".as_bytes());
        assert_eq!(decode("Egrave".as_bytes()).unwrap(), "È".as_bytes());
        assert_eq!(decode("szlig".as_bytes()).unwrap(), "ß".as_bytes());
        assert_eq!(decode("cup".as_bytes()).unwrap(), "∪".as_bytes());
        assert_eq!(decode("piv".as_bytes()).unwrap(), "ϖ".as_bytes());
        assert_eq!(decode("Zeta".as_bytes()).unwrap(), "Ζ".as_bytes());
        assert_eq!(decode("gt".as_bytes()).unwrap(), ">".as_bytes());
        assert_eq!(decode("darr".as_bytes()).unwrap(), "↓".as_bytes());
        assert_eq!(decode("frac14".as_bytes()).unwrap(), "¼".as_bytes());
        assert_eq!(decode("nges".as_bytes()).unwrap(), "≱".as_bytes());
        assert_eq!(decode("frasl".as_bytes()).unwrap(), "⁄".as_bytes());
        assert_eq!(decode("minus".as_bytes()).unwrap(), "−".as_bytes());
        assert_eq!(decode("uarr".as_bytes()).unwrap(), "↑".as_bytes());
        assert_eq!(decode("zeta".as_bytes()).unwrap(), "ζ".as_bytes());
        assert_eq!(decode("Iota".as_bytes()).unwrap(), "Ι".as_bytes());
        assert_eq!(decode("atilde".as_bytes()).unwrap(), "ã".as_bytes());
        assert_eq!(decode("agrave".as_bytes()).unwrap(), "à".as_bytes());
        assert_eq!(decode("Aacute".as_bytes()).unwrap(), "Á".as_bytes());
        assert_eq!(decode("ensp".as_bytes()).unwrap(), " ".as_bytes());
        assert_eq!(decode("mu".as_bytes()).unwrap(), "μ".as_bytes());
        assert_eq!(decode("ocirc".as_bytes()).unwrap(), "ô".as_bytes());
        assert_eq!(decode("deg".as_bytes()).unwrap(), "°".as_bytes());
        assert_eq!(decode("alefsym".as_bytes()).unwrap(), "ℵ".as_bytes());
        assert_eq!(decode("prime".as_bytes()).unwrap(), "′".as_bytes());
        assert_eq!(decode("Gamma".as_bytes()).unwrap(), "Γ".as_bytes());
        assert_eq!(decode("Sigma".as_bytes()).unwrap(), "Σ".as_bytes());
        assert_eq!(decode("sdot".as_bytes()).unwrap(), "⋅".as_bytes());
        assert_eq!(decode("par".as_bytes()).unwrap(), "∥".as_bytes());
        assert_eq!(decode("comet".as_bytes()).unwrap(), "☄".as_bytes());
        assert_eq!(decode("and".as_bytes()).unwrap(), "∧".as_bytes());
        assert_eq!(decode("ndash".as_bytes()).unwrap(), "–".as_bytes());
        assert_eq!(decode("oelig".as_bytes()).unwrap(), "œ".as_bytes());
        assert_eq!(decode("compfn".as_bytes()).unwrap(), "∘".as_bytes());
        assert_eq!(decode("lAarr".as_bytes()).unwrap(), "⇚".as_bytes());
        assert_eq!(decode("Euml".as_bytes()).unwrap(), "Ë".as_bytes());
        assert_eq!(decode("lsaquo".as_bytes()).unwrap(), "‹".as_bytes());
        assert_eq!(decode("thinsp".as_bytes()).unwrap(), " ".as_bytes());
        assert_eq!(decode("omicron".as_bytes()).unwrap(), "ο".as_bytes());
        assert_eq!(decode("thunderstorm".as_bytes()).unwrap(), "☈".as_bytes());
        assert_eq!(decode("cloud".as_bytes()).unwrap(), "☁".as_bytes());
        assert_eq!(decode("mnplus".as_bytes()).unwrap(), "∓".as_bytes());
        assert_eq!(decode("nsup".as_bytes()).unwrap(), "⊅".as_bytes());
        assert_eq!(decode("mdash".as_bytes()).unwrap(), "—".as_bytes());
        assert_eq!(decode("twixt".as_bytes()).unwrap(), "≬".as_bytes());
        assert_eq!(decode("angsph".as_bytes()).unwrap(), "∢".as_bytes());
        assert_eq!(decode("Delta".as_bytes()).unwrap(), "Δ".as_bytes());
        assert_eq!(decode("lambda".as_bytes()).unwrap(), "λ".as_bytes());
        assert_eq!(decode("Eta".as_bytes()).unwrap(), "Η".as_bytes());
        assert_eq!(decode("Theta".as_bytes()).unwrap(), "Θ".as_bytes());
        assert_eq!(decode("crarr".as_bytes()).unwrap(), "↵".as_bytes());
        assert_eq!(decode("Chi".as_bytes()).unwrap(), "Χ".as_bytes());
        assert_eq!(decode("sup3".as_bytes()).unwrap(), "³".as_bytes());
        assert_eq!(decode("snowflake".as_bytes()).unwrap(), "❅".as_bytes());
        assert_eq!(decode("plusdo".as_bytes()).unwrap(), "∔".as_bytes());
        assert_eq!(decode("supe".as_bytes()).unwrap(), "⊇".as_bytes());
        assert_eq!(decode("Lt".as_bytes()).unwrap(), "≪".as_bytes());
        assert_eq!(decode("prop".as_bytes()).unwrap(), "∝".as_bytes());
        assert_eq!(decode("frac34".as_bytes()).unwrap(), "¾".as_bytes());
        assert_eq!(decode("sup2".as_bytes()).unwrap(), "²".as_bytes());
        assert_eq!(decode("reg".as_bytes()).unwrap(), "®".as_bytes());
        assert_eq!(decode("isin".as_bytes()).unwrap(), "∈".as_bytes());
        assert_eq!(decode("sube".as_bytes()).unwrap(), "⊆".as_bytes());
        assert_eq!(decode("rAarr".as_bytes()).unwrap(), "⇛".as_bytes());
        assert_eq!(decode("gl".as_bytes()).unwrap(), "≷".as_bytes());
        assert_eq!(decode("sime".as_bytes()).unwrap(), "≃".as_bytes());
        assert_eq!(decode("nsub".as_bytes()).unwrap(), "⊄".as_bytes());
        assert_eq!(decode("hArr".as_bytes()).unwrap(), "⇔".as_bytes());
        assert_eq!(decode("icirc".as_bytes()).unwrap(), "î".as_bytes());
        assert_eq!(decode("ne".as_bytes()).unwrap(), "≠".as_bytes());
        assert_eq!(decode("ucirc".as_bytes()).unwrap(), "û".as_bytes());
        assert_eq!(decode("coprod".as_bytes()).unwrap(), "∐".as_bytes());
        assert_eq!(decode("oacute".as_bytes()).unwrap(), "ó".as_bytes());
        assert_eq!(decode("cent".as_bytes()).unwrap(), "¢".as_bytes());
        assert_eq!(decode("nsc".as_bytes()).unwrap(), "⊁".as_bytes());
        assert_eq!(decode("cupre".as_bytes()).unwrap(), "≼".as_bytes());
        assert_eq!(decode("lArr".as_bytes()).unwrap(), "⇐".as_bytes());
        assert_eq!(decode("pi".as_bytes()).unwrap(), "π".as_bytes());
        assert_eq!(decode("plusmn".as_bytes()).unwrap(), "±".as_bytes());
        assert_eq!(decode("Phi".as_bytes()).unwrap(), "Φ".as_bytes());
        assert_eq!(decode("infin".as_bytes()).unwrap(), "∞".as_bytes());
        assert_eq!(decode("divide".as_bytes()).unwrap(), "÷".as_bytes());
        assert_eq!(decode("tau".as_bytes()).unwrap(), "τ".as_bytes());
        assert_eq!(decode("frac12".as_bytes()).unwrap(), "½".as_bytes());
        assert_eq!(decode("equiv".as_bytes()).unwrap(), "≡".as_bytes());
        assert_eq!(decode("bump".as_bytes()).unwrap(), "≎".as_bytes());
        assert_eq!(decode("THORN".as_bytes()).unwrap(), "Þ".as_bytes());
        assert_eq!(decode("oline".as_bytes()).unwrap(), "‾".as_bytes());
        assert_eq!(decode("Mu".as_bytes()).unwrap(), "Μ".as_bytes());
        assert_eq!(decode("sub".as_bytes()).unwrap(), "⊂".as_bytes());
        assert_eq!(decode("shy".as_bytes()).unwrap(), "­".as_bytes());
        assert_eq!(decode("nsim".as_bytes()).unwrap(), "≁".as_bytes());
        assert_eq!(decode("thetasym".as_bytes()).unwrap(), "ϑ".as_bytes());
        assert_eq!(decode("Omega".as_bytes()).unwrap(), "Ω".as_bytes());
        assert_eq!(decode("Oslash".as_bytes()).unwrap(), "Ø".as_bytes());
        assert_eq!(decode("ang90".as_bytes()).unwrap(), "∟".as_bytes());
        assert_eq!(decode("iexcl".as_bytes()).unwrap(), "¡".as_bytes());
        assert_eq!(decode("rArr".as_bytes()).unwrap(), "⇒".as_bytes());
        assert_eq!(decode("cedil".as_bytes()).unwrap(), "¸".as_bytes());
        assert_eq!(decode("uacute".as_bytes()).unwrap(), "ú".as_bytes());
        assert_eq!(decode("sup".as_bytes()).unwrap(), "⊃".as_bytes());
        assert_eq!(decode("lE".as_bytes()).unwrap(), "≦".as_bytes());
        assert_eq!(decode("sum".as_bytes()).unwrap(), "∑".as_bytes());
        assert_eq!(decode("ntilde".as_bytes()).unwrap(), "ñ".as_bytes());
        assert_eq!(decode("lceil".as_bytes()).unwrap(), "⌈".as_bytes());
        assert_eq!(decode("bcong".as_bytes()).unwrap(), "≌".as_bytes());
        assert_eq!(decode("mid".as_bytes()).unwrap(), "∣".as_bytes());
        assert_eq!(decode("dArr".as_bytes()).unwrap(), "⇓".as_bytes());
        assert_eq!(decode("sigma".as_bytes()).unwrap(), "σ".as_bytes());
        assert_eq!(decode("nsime".as_bytes()).unwrap(), "≄".as_bytes());
        assert_eq!(decode("Xi".as_bytes()).unwrap(), "Ξ".as_bytes());
        assert_eq!(decode("sc".as_bytes()).unwrap(), "≻".as_bytes());
        assert_eq!(decode("Lambda".as_bytes()).unwrap(), "Λ".as_bytes());
        assert_eq!(decode("oslash".as_bytes()).unwrap(), "ø".as_bytes());
        assert_eq!(decode("forall".as_bytes()).unwrap(), "∀".as_bytes());
        assert_eq!(decode("umbrella".as_bytes()).unwrap(), "☂".as_bytes());
        assert_eq!(decode("uArr".as_bytes()).unwrap(), "⇑".as_bytes());
        assert_eq!(decode("diams".as_bytes()).unwrap(), "♦".as_bytes());
        assert_eq!(decode("iquest".as_bytes()).unwrap(), "¿".as_bytes());
        assert_eq!(decode("eta".as_bytes()).unwrap(), "η".as_bytes());
        assert_eq!(decode("gamma".as_bytes()).unwrap(), "γ".as_bytes());
        assert_eq!(decode("iuml".as_bytes()).unwrap(), "ï".as_bytes());
        assert_eq!(decode("middot".as_bytes()).unwrap(), "·".as_bytes());
        assert_eq!(decode("gE".as_bytes()).unwrap(), "≧".as_bytes());
        assert_eq!(decode("dagger".as_bytes()).unwrap(), "†".as_bytes());
        assert_eq!(decode("weierp".as_bytes()).unwrap(), "℘".as_bytes());
        assert_eq!(decode("ouml".as_bytes()).unwrap(), "ö".as_bytes());
        assert_eq!(decode("perp".as_bytes()).unwrap(), "⊥".as_bytes());
        assert_eq!(decode("curren".as_bytes()).unwrap(), "¤".as_bytes());
        assert_eq!(decode("amp".as_bytes()).unwrap(), "&".as_bytes());
        assert_eq!(decode("iota".as_bytes()).unwrap(), "ι".as_bytes());
        assert_eq!(decode("quot".as_bytes()).unwrap(), "\"".as_bytes());
        assert_eq!(decode("ang".as_bytes()).unwrap(), "∠".as_bytes());
        assert_eq!(decode("Iuml".as_bytes()).unwrap(), "Ï".as_bytes());
        assert_eq!(decode("spades".as_bytes()).unwrap(), "♠".as_bytes());
        assert_eq!(decode("ge".as_bytes()).unwrap(), "≥".as_bytes());
        assert_eq!(decode("image".as_bytes()).unwrap(), "ℑ".as_bytes());
        assert_eq!(decode("psi".as_bytes()).unwrap(), "ψ".as_bytes());
        assert_eq!(decode("Eacute".as_bytes()).unwrap(), "É".as_bytes());
        assert_eq!(decode("uuml".as_bytes()).unwrap(), "ü".as_bytes());
        assert_eq!(decode("radic".as_bytes()).unwrap(), "√".as_bytes());
        assert_eq!(decode("ni".as_bytes()).unwrap(), "∋".as_bytes());
        assert_eq!(decode("bull".as_bytes()).unwrap(), "•".as_bytes());
        assert_eq!(decode("times".as_bytes()).unwrap(), "×".as_bytes());
        assert_eq!(decode("AElig".as_bytes()).unwrap(), "Æ".as_bytes());
        assert_eq!(decode("ordm".as_bytes()).unwrap(), "º".as_bytes());
        assert_eq!(decode("prsim".as_bytes()).unwrap(), "≾".as_bytes());
        assert_eq!(decode("bepsi".as_bytes()).unwrap(), "∍".as_bytes());
        assert_eq!(decode("epsis".as_bytes()).unwrap(), "∊".as_bytes());
        assert_eq!(decode("vArr".as_bytes()).unwrap(), "⇕".as_bytes());
        assert_eq!(decode("ngt".as_bytes()).unwrap(), "≯".as_bytes());
        assert_eq!(decode("part".as_bytes()).unwrap(), "∂".as_bytes());
        assert_eq!(decode("otimes".as_bytes()).unwrap(), "⊗".as_bytes());
        assert_eq!(decode("micro".as_bytes()).unwrap(), "µ".as_bytes());
        assert_eq!(decode("raquo".as_bytes()).unwrap(), "»".as_bytes());
        assert_eq!(decode("Ocirc".as_bytes()).unwrap(), "Ô".as_bytes());
        assert_eq!(decode("macr".as_bytes()).unwrap(), "¯".as_bytes());
        assert_eq!(decode("Upsilon".as_bytes()).unwrap(), "Υ".as_bytes());
        assert_eq!(decode("Auml".as_bytes()).unwrap(), "Ä".as_bytes());
        assert_eq!(decode("sup1".as_bytes()).unwrap(), "¹".as_bytes());
        assert_eq!(decode("iacute".as_bytes()).unwrap(), "í".as_bytes());
        assert_eq!(decode("ETH".as_bytes()).unwrap(), "Ð".as_bytes());
        assert_eq!(decode("Icirc".as_bytes()).unwrap(), "Î".as_bytes());
        assert_eq!(decode("or".as_bytes()).unwrap(), "∨".as_bytes());
        assert_eq!(decode("sigmaf".as_bytes()).unwrap(), "ς".as_bytes());
        assert_eq!(decode("bumpe".as_bytes()).unwrap(), "≏".as_bytes());
        assert_eq!(decode("phi".as_bytes()).unwrap(), "φ".as_bytes());
        assert_eq!(decode("pr".as_bytes()).unwrap(), "≺".as_bytes());
        assert_eq!(decode("Ucirc".as_bytes()).unwrap(), "Û".as_bytes());
        assert_eq!(decode("beta".as_bytes()).unwrap(), "β".as_bytes());
        assert_eq!(decode("aring".as_bytes()).unwrap(), "å".as_bytes());
        assert_eq!(decode("lfloor".as_bytes()).unwrap(), "⌊".as_bytes());
        assert_eq!(decode("Epsilon".as_bytes()).unwrap(), "Ε".as_bytes());
        assert_eq!(decode("upsih".as_bytes()).unwrap(), "ϒ".as_bytes());
        assert_eq!(decode("thorn".as_bytes()).unwrap(), "þ".as_bytes());
        assert_eq!(decode("Oacute".as_bytes()).unwrap(), "Ó".as_bytes());
        assert_eq!(decode("rarrw".as_bytes()).unwrap(), "⇝".as_bytes());
        assert_eq!(decode("larr".as_bytes()).unwrap(), "←".as_bytes());
        assert_eq!(decode("aelig".as_bytes()).unwrap(), "æ".as_bytes());
        assert_eq!(decode("gnE".as_bytes()).unwrap(), "≩".as_bytes());
        assert_eq!(decode("brvbar".as_bytes()).unwrap(), "¦".as_bytes());
        assert_eq!(decode("asympeq".as_bytes()).unwrap(), "≍".as_bytes());
        assert_eq!(decode("int".as_bytes()).unwrap(), "∫".as_bytes());
        assert_eq!(decode("ccedil".as_bytes()).unwrap(), "ç".as_bytes());
        assert_eq!(decode("npar".as_bytes()).unwrap(), "∦".as_bytes());
        assert_eq!(decode("lsquo".as_bytes()).unwrap(), "‘".as_bytes());
        assert_eq!(decode("harr".as_bytes()).unwrap(), "↔".as_bytes());
        assert_eq!(decode("Rho".as_bytes()).unwrap(), "Ρ".as_bytes());
        assert_eq!(decode("pound".as_bytes()).unwrap(), "£".as_bytes());
        assert_eq!(decode("apos".as_bytes()).unwrap(), "'".as_bytes());
        assert_eq!(decode("real".as_bytes()).unwrap(), "ℜ".as_bytes());
        assert_eq!(decode("hellip".as_bytes()).unwrap(), "…".as_bytes());
        assert_eq!(decode("Ouml".as_bytes()).unwrap(), "Ö".as_bytes());
        assert_eq!(decode("euml".as_bytes()).unwrap(), "ë".as_bytes());
        assert_eq!(decode("uml".as_bytes()).unwrap(), "¨".as_bytes());
        assert_eq!(decode("Kappa".as_bytes()).unwrap(), "Κ".as_bytes());
        assert_eq!(decode("rceil".as_bytes()).unwrap(), "⌉".as_bytes());
        assert_eq!(decode("notni".as_bytes()).unwrap(), "∌".as_bytes());
        assert_eq!(decode("ugrave".as_bytes()).unwrap(), "ù".as_bytes());
        assert_eq!(decode("acirc".as_bytes()).unwrap(), "â".as_bytes());
        assert_eq!(decode("laquo".as_bytes()).unwrap(), "«".as_bytes());
        assert_eq!(decode("ncong".as_bytes()).unwrap(), "≇".as_bytes());
        assert_eq!(decode("para".as_bytes()).unwrap(), "¶".as_bytes());
        assert_eq!(decode("asymp".as_bytes()).unwrap(), "≈".as_bytes());
        assert_eq!(decode("Agrave".as_bytes()).unwrap(), "À".as_bytes());
        assert_eq!(decode("Pi".as_bytes()).unwrap(), "Π".as_bytes());
        assert_eq!(decode("aacute".as_bytes()).unwrap(), "á".as_bytes());
        assert_eq!(decode("gsim".as_bytes()).unwrap(), "≳".as_bytes());
        assert_eq!(decode("rfloor".as_bytes()).unwrap(), "⌋".as_bytes());
        assert_eq!(decode("rarr".as_bytes()).unwrap(), "→".as_bytes());
        assert_eq!(decode("ecirc".as_bytes()).unwrap(), "ê".as_bytes());
        assert_eq!(decode("delta".as_bytes()).unwrap(), "δ".as_bytes());
        assert_eq!(decode("Ograve".as_bytes()).unwrap(), "Ò".as_bytes());
        assert_eq!(decode("there4".as_bytes()).unwrap(), "∴".as_bytes());
        assert_eq!(decode("Prime".as_bytes()).unwrap(), "″".as_bytes());
        assert_eq!(decode("sect".as_bytes()).unwrap(), "§".as_bytes());
        assert_eq!(decode("empty".as_bytes()).unwrap(), "∅".as_bytes());
        assert_eq!(decode("Omicron".as_bytes()).unwrap(), "Ο".as_bytes());
        assert_eq!(decode("tilde".as_bytes()).unwrap(), "˜".as_bytes());
        assert_eq!(decode("fnof".as_bytes()).unwrap(), "ƒ".as_bytes());
        assert_eq!(decode("eth".as_bytes()).unwrap(), "ð".as_bytes());
        assert_eq!(decode("ordf".as_bytes()).unwrap(), "ª".as_bytes());
        assert_eq!(decode("zwj".as_bytes()).unwrap(), "‍".as_bytes());
        assert_eq!(decode("nmid".as_bytes()).unwrap(), "∤".as_bytes());
        assert_eq!(decode("rho".as_bytes()).unwrap(), "ρ".as_bytes());
        assert_eq!(decode("auml".as_bytes()).unwrap(), "ä".as_bytes());
        assert_eq!(decode("lnE".as_bytes()).unwrap(), "≨".as_bytes());
        assert_eq!(decode("zwnj".as_bytes()).unwrap(), "‌".as_bytes());
        assert_eq!(decode("Uacute".as_bytes()).unwrap(), "Ú".as_bytes());
        assert_eq!(decode("yuml".as_bytes()).unwrap(), "ÿ".as_bytes());
        assert_eq!(decode("Aring".as_bytes()).unwrap(), "Å".as_bytes());
        assert_eq!(decode("lsim".as_bytes()).unwrap(), "≲".as_bytes());
        assert_eq!(decode("nap".as_bytes()).unwrap(), "≉".as_bytes());
        assert_eq!(decode("Scaron".as_bytes()).unwrap(), "Š".as_bytes());
        assert_eq!(decode("acute".as_bytes()).unwrap(), "´".as_bytes());
        assert_eq!(decode("yacute".as_bytes()).unwrap(), "ý".as_bytes());
        assert_eq!(decode("lowast".as_bytes()).unwrap(), "∗".as_bytes());
        assert_eq!(decode("egrave".as_bytes()).unwrap(), "è".as_bytes());
        assert_eq!(decode("Acirc".as_bytes()).unwrap(), "Â".as_bytes());
        assert_eq!(decode("euro".as_bytes()).unwrap(), "€".as_bytes());
        assert_eq!(decode("Gt".as_bytes()).unwrap(), "≫".as_bytes());
        assert_eq!(decode("igrave".as_bytes()).unwrap(), "ì".as_bytes());
        assert_eq!(decode("cap".as_bytes()).unwrap(), "∩".as_bytes());
        assert_eq!(decode("Otilde".as_bytes()).unwrap(), "Õ".as_bytes());
        assert_eq!(decode("scsim".as_bytes()).unwrap(), "≿".as_bytes());
        assert_eq!(decode("Igrave".as_bytes()).unwrap(), "Ì".as_bytes());
        assert_eq!(decode("exist".as_bytes()).unwrap(), "∃".as_bytes());
        assert_eq!(decode("nu".as_bytes()).unwrap(), "ν".as_bytes());
        assert_eq!(decode("omega".as_bytes()).unwrap(), "ω".as_bytes());
        assert_eq!(decode("snowman".as_bytes()).unwrap(), "☃".as_bytes());
        assert_eq!(decode("cong".as_bytes()).unwrap(), "≅".as_bytes());
        assert_eq!(decode("Ccedil".as_bytes()).unwrap(), "Ç".as_bytes());
        assert_eq!(decode("rang".as_bytes()).unwrap(), "〉".as_bytes());
        assert_eq!(decode("setmn".as_bytes()).unwrap(), "∖".as_bytes());
        assert_eq!(decode("rdquo".as_bytes()).unwrap(), "”".as_bytes());
        assert_eq!(decode("yen".as_bytes()).unwrap(), "¥".as_bytes());
        assert_eq!(decode("ograve".as_bytes()).unwrap(), "ò".as_bytes());
        assert_eq!(decode("Psi".as_bytes()).unwrap(), "Ψ".as_bytes());
        assert_eq!(decode("lt".as_bytes()).unwrap(), "<".as_bytes());
        assert_eq!(decode("epsilon".as_bytes()).unwrap(), "ε".as_bytes());
        assert_eq!(decode("alpha".as_bytes()).unwrap(), "α".as_bytes());
        assert_eq!(decode("sim".as_bytes()).unwrap(), "∼".as_bytes());
        assert_eq!(decode("Uuml".as_bytes()).unwrap(), "Ü".as_bytes());
        assert_eq!(decode("not".as_bytes()).unwrap(), "¬".as_bytes());
        assert_eq!(decode("nbsp".as_bytes()).unwrap(), " ".as_bytes());
        assert_eq!(decode("circ".as_bytes()).unwrap(), "ˆ".as_bytes());
        assert_eq!(decode("copy".as_bytes()).unwrap(), "©".as_bytes());
        assert_eq!(decode("chi".as_bytes()).unwrap(), "χ".as_bytes());
        assert_eq!(decode("Beta".as_bytes()).unwrap(), "Β".as_bytes());
        assert_eq!(decode("Alpha".as_bytes()).unwrap(), "Α".as_bytes());
        assert_eq!(decode("Ntilde".as_bytes()).unwrap(), "Ñ".as_bytes());
        assert_eq!(decode("Dagger".as_bytes()).unwrap(), "‡".as_bytes());
        assert_eq!(decode("sbquo".as_bytes()).unwrap(), "‚".as_bytes());
        assert_eq!(decode("eacute".as_bytes()).unwrap(), "é".as_bytes());
        assert_eq!(decode("nles".as_bytes()).unwrap(), "≰".as_bytes());
        assert_eq!(decode("comp".as_bytes()).unwrap(), "∁".as_bytes());
        assert_eq!(decode("Yacute".as_bytes()).unwrap(), "Ý".as_bytes());
        assert_eq!(decode("Iacute".as_bytes()).unwrap(), "Í".as_bytes());
        assert_eq!(decode("otilde".as_bytes()).unwrap(), "õ".as_bytes());

    }
}
