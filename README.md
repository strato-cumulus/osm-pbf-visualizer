# osm-pbf-visualizer

A visualizer for OpenStreetMap PBF files, written from grounds up as a learning exercise for modern C++, Protobuf and SDL3.

## Requirements

- SDL3: at the time of writing it was still in beta, I've been using `3.1.6+ds-2` from Debian experimental
- protobuf
- zlib

## Running

Download an OSM PBF dump first. I've been using city dumps from [Geofabrik](https://download.geofabrik.de/europe/germany/berlin.html).

```bash
make
bin/osm path-to-your\.osm.pbf
```
