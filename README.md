# simple_midi_to_rawvideo

midi visualiser to rawvideo then encode with ffmpeg

Usage:
see mvid.sh

Building:
> g++ *.cpp -o main

this program uses mmap to load midi file.
it also uses endian.h for be16toh and be32toh
