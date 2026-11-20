# Alcance y método

## Qué significa este proyecto

El cliente oficial de Java Edition no se puede convertir a C++ mediante una compilación: su código fuente no está publicado como un port C++ por Mojang. Horizon resolvió algo distinto: una implementación alternativa, hecha desde cero en Rust, comparada por comportamiento con el juego oficial. Este repositorio inicia ese mismo tipo de proyecto, pero con C++20.

No se toma ni traduce código de Horizon, Mojang u otros clientes. No se extraen fuentes del JAR. El JAR y los assets oficiales quedan fuera del repositorio y bajo control del usuario. Al alcanzar una etapa que requiera comparar con el juego, el arnés ejecutará la instalación que el usuario ya posee y almacenará únicamente fixtures/resultados permitidos, no los binarios del juego.

## Primera pieza: NBT Java

Se implementa un parser del formato NBT big-endian con límites configurables para profundidad, cantidad de elementos y bytes. Los tests generan pequeños flujos NBT desde cero y verifican tags escalares, listas, compounds y entradas malformadas/truncadas. Esto no significa que el programa ya pueda cargar un mundo: todavía falta Region/Anvil, compresión, lectura de chunks y conversión a entidades del motor.

## Etapas siguientes

1. NBT robusto y pruebas de formato.
2. Contenedor Region/Anvil, lectura de chunks y pruebas con fixtures propios y mundos que el usuario autorice usar localmente.
3. Modelo de bloques y datos de mundo/versiones con casos observables comparados contra Minecraft 26.3.
4. Generación de terreno por etapas y semillas fijadas, usando un oráculo local del juego oficial cuando sea legal y técnicamente posible.
5. Protocolos/servidor, simulación, renderizador y controles en módulos desacoplados.
6. Cliente ejecutable solo después de integrar y validar slices suficientes.

Cada etapa debe indicar qué está implementado, qué coincide con el oráculo, qué falta y qué comandos/pruebas se ejecutaron. El CI prueba compilación y pruebas unitarias propias; no se presentará como prueba de paridad del juego.
