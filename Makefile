
install:
	g++ -g -o osm -DSDL_MAIN_USE_CALLBACKS=1 \
		osm_unpack/*.pb.cc osm_unpack/*.cpp \
		visualizer/renderer.cpp \
		main.cpp \
		-lprotobuf -lz -lSDL3 -lstdc++
