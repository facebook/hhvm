<?php
// Check the initial settings for all assert_options 

//Using assert_options;
echo "Initial values: assert_options(ASSERT_ACTIVE) => [".assert_options(ASSERT_ACTIVE)."]\n";
echo "Initial values: assert_options(ASSERT_WARNING) => [".assert_options(ASSERT_WARNING)."]\n";
echo "Initial values: assert_options(ASSERT_BAIL) => [".assert_options(ASSERT_BAIL)."]\n";
echo "Initial values: assert_options(ASSERT_QUIET_EVAL) => [".assert_options(ASSERT_QUIET_EVAL)."]\n";
echo "Initial values: assert_options(ASSERT_CALLBACK) => [".assert_options(ASSERT_CALLBACK)."]\n";

//Using ini.get;
echo "Initial values: ini.get(\"assert.active\") => [".ini_get("assert.active")."]\n";
echo "Initial values: ini.get(\"assert.warning\") => [".ini_get("assert.warning")."]\n";
echo "Initial values: ini.get(\"assert.bail\") => [".ini_get("assert.bail")."]\n";
echo "Initial values: ini.get(\"assert.quiet_eval\") => [".ini_get("assert.quiet_eval")."]\n";
echo "Initial values: ini.get(\"assert.callback\") => [".ini_get("assert.callback")."]\n\n";
