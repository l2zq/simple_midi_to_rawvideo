#!/bin/sh

if [ $# -lt 1 ] ; then
  echo $0 filename [width height] [fps] [height] [max_polyphony]
  exit 1
fi

FN="$1"
FW=640
FH=480
FPS=30
VPH=1
MAX=32768

if [ $# -ge 3 ] ; then
  FW=$2
  FH=$3
fi
if [ $# -ge 4 ] ; then
  FPS=$4
fi
if [ $# -ge 5 ] ; then
  VPH=$5
fi
if [ $# -ge 6 ] ; then
  MAX=$6
fi

./main $FN $FW $FH $FPS $VPH $MAX | ffplay -f rawvideo -pixel_format bgra -video_size ${FW}x${FH} -framerate ${FPS} -i -
