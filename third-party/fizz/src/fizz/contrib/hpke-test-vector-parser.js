#!/bin/env node

// This parses HPKE test vectors into test cases for HpkeTest.cpp

const fs = require('fs');
if (process.argv.length != 3) {
  console.log("usage: " + process.argv[1] + " <json>");
  process.exit(1);
}

const jsonStr = fs.readFileSync(process.argv[2]);
const vec = JSON.parse(jsonStr);

vec.forEach(function(entry) {
  if (entry.kem_id == 0x21 || entry.aead_id == 0xFFFF ||
      entry.encryptions.length == 0) {
    // Unsupported params
    return;
  }
  console.log("Params{");
  switch (entry.mode) {
    case 0:
      console.log("    Mode::Base,");
      break;
    case 1:
      console.log("    Mode::Psk,");
      break;
    case 2:
      console.log("    Mode::Auth,");
      break;
    case 3:
      console.log("    Mode::AuthPsk,");
      break;
  }
  switch (entry.kem_id) {
    case 0x10:
      console.log("    NamedGroup::secp256r1,");
      break;
    case 0x11:
      console.log("    NamedGroup::secp384r1,");
      break;
    case 0x12:
      console.log("    NamedGroup::secp521r1,");
      break;
    case 0x20:
      console.log("    NamedGroup::x25519,");
      break;
  }
  switch (entry.kdf_id) {
    case 1:
      console.log("    HashFunction::Sha256,");
      break;
    case 2:
      console.log("    HashFunction::Sha384,");
      break;
    case 3:
      console.log("    HashFunction::Sha512,");
      break;
  }
  switch (entry.aead_id) {
    case 1:
      console.log("    CipherSuite::TLS_AES_128_GCM_SHA256,");
      break;
    case 2:
      console.log("    CipherSuite::TLS_AES_256_GCM_SHA384,");
      break;
    case 3:
      console.log("    CipherSuite::TLS_CHACHA20_POLY1305_SHA256,");
      break;
  }
  console.log("    \"" + entry.shared_secret + "\",");
  console.log("    \"" + entry.info + "\",");
  console.log("    \"" + entry.skEm + "\",");
  console.log("    \"" + entry.pkEm + "\",");
  console.log("    \"" + entry.skRm + "\",");
  console.log("    \"" + entry.pkRm + "\",");
  if (entry.hasOwnProperty("skSm")) {
    console.log("    \"" + entry.skSm + "\",");
    console.log("    \"" + entry.pkSm + "\",");
  } else {
    console.log("    \"\",");
    console.log("    \"\",");
  }

  if (entry.hasOwnProperty("psk")) {
    console.log("    \"" + entry.psk + "\",");
    console.log("    \"" + entry.psk_id + "\",");
  } else {
    console.log("    \"\",");
    console.log("    \"\",");
  }
  console.log("    \"" + entry.key + "\",");
  console.log("    \"" + entry.base_nonce + "\",");
  console.log("    \"" + entry.encryptions[0].ct + "\",");
  console.log("    \"" + entry.exporter_secret + "\",");
  console.log("    {");
  console.log("        \"" + entry.exports[0].exported_value + "\",");
  console.log("        \"" + entry.exports[1].exported_value + "\",");
  console.log("        \"" + entry.exports[2].exported_value + "\",");
  console.log("    }");
  console.log("},");
});
