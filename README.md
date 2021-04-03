# esp-config-state

![platformio build](https://github.com/mdvorak-iot/esp-config-state/workflows/platformio%20build/badge.svg)

Simple C++ serialization library for application configurations, with JSON
and [NVS](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/storage/nvs_flash.html) support.

Uses [RapidJSON](https://github.com/Tencent/rapidjson/) library for serialization.

## RapidJSON dependency

Library must be provided by a parent project.

These definitions are mandatory, otherwise rapidjson allocates whole 64KB on heap, when it needs only few hundred bytes:

```
-D RAPIDJSON_HAS_STDSTRING=1 
-D RAPIDJSON_ALLOCATOR_DEFAULT_CHUNK_CAPACITY=1024
```

For CMake, set them at ideally at library import, via

```cmake
target_compile_definitions(rapidjson INTERFACE RAPIDJSON_HAS_STDSTRING=1 RAPIDJSON_ALLOCATOR_DEFAULT_CHUNK_CAPACITY=1024)
```
