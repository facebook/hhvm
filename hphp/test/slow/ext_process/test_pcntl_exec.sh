#!/bin/sh
if [ $name != "value" ]; then
    echo "======================================"
    echo "TestExtProcess::test_pcntl_exec FAILED"
    echo "======================================"
else
    echo "========================================="
    echo "TestExtProcess::test_pcntl_exec SUCCEEDED"
    echo "========================================="
fi
