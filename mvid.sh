#!/bin/sh

if [ $# -lt 1 ]; then
  echo $0 filename [width height = 640 480] [fps = 30] [height = 1] [max_polyphony = 32768] [output filename] [audio file]
  exit 1
fi

FN="$1"
FW=640
FH=480
FPS=30
VPH=1
MAX=32768

if [ $# -ge 3 ]; then
  FW=$2
  FH=$3
fi
if [ $# -ge 4 ]; then
  FPS=$4
fi
if [ $# -ge 5 ]; then
  VPH=$5
fi
if [ $# -ge 6 ]; then
  MAX=$6
fi

FFARG="-c:v h264_nvenc -pix_fmt yuv420p -b:v 19000k"
OUTFN="${FN}-${FW}x${FH}@${FPS}-${VPH}.mkv"

if [ $# -ge 7 ]; then
  OUTFN="$7"
fi
if [ $# -ge 8 ]; then
  AUDIO="$8"
  FFARG="-i $AUDIO $FFARG -c:a aac -b:a 320k"
fi

shift 1

./main "${FN}" $* | prime-run ffmpeg -f rawvideo -pixel_format bgra -video_size ${FW}x${FH} -framerate ${FPS} -i - ${FFARG} -y "${OUTFN}"
