# esp-config-state

![platformio build](https://github.com/mdvorak-iot/esp-config-state/workflows/platformio%20build/badge.svg)

TODO Description

## rapidjson dependency

TODO

These definitions are mandatory, otherwise rapidjson allocates whole 64KB on heap, when it needs only few bytes:

```
-D RAPIDJSON_HAS_STDSTRING=1 
-D RAPIDJSON_ALLOCATOR_DEFAULT_CHUNK_CAPACITY=1024
```

For CMake, set them at ideally at root project via
```cmake
add_compile_definitions(RAPIDJSON_HAS_STDSTRING=1 RAPIDJSON_ALLOCATOR_DEFAULT_CHUNK_CAPACITY=1024)
```
