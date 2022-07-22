#!/bin/bash

# positional arguments: 1 - port number, 2 - episode number

PROJECT_DIR=../rl_impl

source $PROJECT_DIR/venv/bin/activate
python $PROJECT_DIR/main.py localhost $1
