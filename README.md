# Minecraft C++ Client

Proyecto de reimplementación **clean-room** en C++20 del comportamiento observable de Minecraft: Java Edition, empezando por una base nativa independiente. La meta es aprender y reproducir comportamiento mediante especificaciones, fixtures y pruebas diferenciales contra una instalación oficial que el usuario ya posea; no es un port del código fuente de Mojang.

**Estado: prototipo inicial.** Este repositorio todavía no es un cliente de Minecraft ni abre mundos/juego. No se afirma compatibilidad, paridad, renderizado o capacidad de conectarse a servidores. La primera pieza es un lector seguro de NBT Java Edition, formato usado por datos del juego. No contiene ni distribuye código, JAR, texturas u otros archivos de Minecraft.

## Construir y probar

Requiere CMake 3.20+ y un compilador C++20. No tiene dependencias externas.

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

## Enfoque

- Implementar slices pequeños en C++ limpio, separados por responsabilidad.
- Usar la edición Java oficial instalada por el usuario como oráculo de comportamiento cuando se añadan pruebas diferenciales; nunca copiar ni redistribuir sus archivos.
- Mantener fixtures y resultados reproducibles; cada resultado se etiqueta con la versión exacta de Minecraft y las condiciones de ejecución.
- No marcar una característica como compatible hasta tener implementación, tests y evidencia real.

Consulta [el plan y límites](docs/ROADMAP.md). El nombre Minecraft y sus marcas pertenecen a Mojang/Microsoft; este es un proyecto independiente, no oficial ni afiliado.
