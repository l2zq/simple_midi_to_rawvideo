# simple_midi_to_rawvideo
./main x.mid 1280 720 60 1.0 | ffmpeg -f rawvideo -pixel_format abgr -video_size 1280x720 -framerate 60 -i - ......

Building:
> g++ *.cpp -o main

it uses mmap
