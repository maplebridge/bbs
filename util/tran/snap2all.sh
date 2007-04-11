#!/bin/sh

echo "Start to convert..."
echo "Convert usr"
./snap2usr
echo "Convert usr's MF"
./snap2MF
echo "Convert brd"
./snap2brd
echo "Convert brd's .DIR"
./snap2hdr
echo "Convert class"
./snap2class
echo "Convert gem"
./snap2gem
./snap2gem2

