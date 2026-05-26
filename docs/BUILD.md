# Guia de compilação

## Requisitos

- GCC 12+ ou Clang 16+
- CMake 3.20+
- libgrpc++, protobuf, libzmq3-dev

## Windows (MSVC)

```powershell
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

## Flags recomendadas (GCC)

```
-O3 -march=native -flto -DNDEBUG -fno-rtti
```

Desabilitar exceções no hot path reduz código gerado; use `expected` ou códigos de erro.
