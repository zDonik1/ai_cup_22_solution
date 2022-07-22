#!/bin/bash

# positional arguments: 1 - config file, 2 - episode number

../game_server/aicup22 --config $1 --save-replay episode_$2/replay --save-results episode_$2/result --batch-mode &> episode_$2/server.out
