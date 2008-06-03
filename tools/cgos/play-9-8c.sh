#!/bin/bash

# Script for playing Fuego on 9x9 CGOS on a machine with 8 cores / 8 GB

FUEGO="../../build/gmake/build/release/fuego"
VERSION=$(cd ../..; svnversion) || exit 1
NAME=Fuego-$VERSION-8c

echo "Use name '$NAME'? (y/n)"
read ANSWER
[[ "$ANSWER" == "y" ]] || exit 1
echo "Enter CGOS password for this player:"
read PASSWORD

GAMES_DIR="games/$NAME"
mkdir -p "$GAMES_DIR"

cat <<EOF >config-9-8c.gtp
# Best performance settings for CGOS
# Uses the time limits, therefore the performance depends on the hardware.

go_param debug_to_comment 1
go_param auto_save $GAMES_DIR/$NAME-

# UCT player parameters
# A node size is currently 64 bytes on a 64-bit machine, so a main memory
# of 7 GB can contain two trees (of the search and the init tree used for
# reuse_subtree) of about 100.000.000 nodes each
uct_param_player max_nodes 100000000
uct_param_player max_games 999999999
uct_param_player ignore_clock 0
uct_param_player reuse_subtree 1
uct_param_player ponder 1

# Set CGOS rules (Tromp-Taylor, positional superko)
go_rules cgos

book_load ../../book/book.dat

sg_param time_mode real
uct_param_search number_threads 8
# lock_free currently causes crashes
uct_param_search lock_free 0
EOF

# Append 2>/dev/stderr to invocation, otherwise cgos3.tcl will not pass
# through stderr of the Go program
./cgos3.patched.tcl Fuego-$VERSION-8c "$PASSWORD" \
  "$FUEGO -config config-9-8c.gtp 2>/dev/stderr" \
  gracefully_exit_server-9-8c
