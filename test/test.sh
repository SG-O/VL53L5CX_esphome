#!/bin/bash

source ../.venv/bin/activate

mapfile -t yaml_files < <(find . -not -path '*/.esphome/*' -name "*.yaml" | sort)

for file in "${yaml_files[@]}"; do
    echo "Testing configuration $file..."
    if ! esphome config "$file"; then
        echo "ERROR: Configuration test failed for $file"
        exit 1
    fi
done

for file in "${yaml_files[@]}"; do
    esphome clean "$file"
done

for file in "${yaml_files[@]}"; do
    echo "Compiling $file..."
    if ! esphome compile "$file"; then
        echo "ERROR: Compilation failed for $file"
        exit 1
    fi
done
