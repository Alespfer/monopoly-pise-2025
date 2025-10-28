<div align="right">
  <a href="./README.md">English</a> |
  <a href="./README_fr.md">Français</a> |
  <a href="./README_es.md">Español</a>
</div>

# Monopoly de Consola en C

![License](https://img.shields.io/badge/license-MIT-blue.svg)
![Language](https://img.shields.io/badge/language-C-green.svg)
![Platform](https://img.shields.io/badge/platform-Linux%20%7C%20macOS-lightgrey.svg)

Una simulación completa y funcional del clásico juego de mesa Monopoly, desarrollada en C y basada en texto. Este proyecto implementa todas las reglas oficiales, incluyendo la gestión de propiedades (compra, construcción, hipotecas), subastas, negociaciones entre jugadores y múltiples modos de juego, todo dentro de una terminal tipo Unix.

![Tablero de juego principal](./screenshots/screenshot-board.png)

## Tabla de Contenidos

- [Sobre el Proyecto](#sobre-el-proyecto)
- [Características Principales](#características-principales)
- [Arquitectura y Diseño](#arquitectura-y-diseño)
- [Stack Tecnológico](#stack-tecnológico)
- [Instalación y Uso](#instalación-y-uso)
- [Licencia](#licencia)
- [Contacto](#contacto)

## Sobre el Proyecto

Este proyecto fue desarrollado como parte de un programa universitario en la Université Paris Cité. El objetivo principal era implementar desde cero un motor de lógica de juego complejo en C, sin dependencias externas, centrándose en estructuras de datos robustas, gestión de estado e interacción con el usuario en un entorno de consola.

## Características Principales

-   🎲 **Motor de Juego Completo:** Gestiona los turnos de los jugadores, lanzamientos de dados, movimiento, reglas de la cárcel y el robo de cartas ("Suerte" y "Caja de Comunidad").
-   🏛️ **Gestión Inmobiliaria Avanzada:** Compra de propiedades, construcción de casas y hoteles (respetando la regla de construcción equilibrada), cobro de alquileres e hipoteca de activos para obtener fondos.
-   🤝 **Interacción Rica entre Jugadores:** Incluye un sistema de **subasta** automática cuando un jugador se niega a comprar una propiedad, y un módulo de **negociación** completo para intercambiar propiedades, dinero y tarjetas "Queda libre de la cárcel".
-   🏆 **Tres Modos de Juego Distintos:**
    1.  **Clásico:** El último jugador que no haya quebrado gana la partida.
    2.  **Turnos Limitados:** El jugador más rico gana después de un número predeterminado de turnos.
    3.  **Objetivo Monetario:** El primer jugador en alcanzar un patrimonio neto objetivo gana.
-   🎨 **Interfaz ASCII Elaborada:** Un tablero de juego completo, en color y actualizado dinámicamente, renderizado en ASCII, que muestra los grupos de propiedades, los edificios y las posiciones de las fichas de los jugadores en tiempo real.
-   💾 **Persistencia de Partidas:** Guarda el estado completo de cualquier partida en curso en un archivo y cárgalo más tarde para reanudar el juego.
-   📜 **Tabla de Mejores Puntuaciones:** Las 10 mejores puntuaciones se guardan y se muestran de forma persistente, fomentando la rejugabilidad.

## Arquitectura y Diseño

La aplicación está construida como un **programa C monolítico** (todo el código fuente reside en un único archivo `monopoly.c`). El estado del juego se gestiona a través de un conjunto de variables globales y estructuras de datos `struct` (`Joueur`, `CasePlateau`, `Carte`), un enfoque pragmático para simplificar el flujo de información entre funciones en este contexto.

La persistencia de los datos se maneja mediante un **mecanismo de serialización personalizado basado en texto**. Las partidas guardadas y las puntuaciones más altas se escriben en archivos `.txt` portátiles, lo que permite que una sesión de juego se pueda mover y reanudar fácilmente en otra máquina compatible.

## Stack Tecnológico

-   **Lenguaje:** C (Estándar C11)
-   **Compilador:** GCC / Clang
-   **Entorno:** Terminal tipo Unix (Linux, macOS)
-   **Dependencias:** Ninguna (solo bibliotecas estándar de C).

## Instalación y Uso

Para obtener una copia local y ponerla en marcha, sigue estos sencillos pasos.

1.  **Clona el repositorio:**
    ```sh
    git clone https://github.com/Alespfer/monopoly-pise-2025.git
    ```
2.  **Navega al directorio del proyecto:**
    ```sh
    cd monopoly-pise-2025
    ```
3.  **Compila el programa:**
    El proyecto no tiene dependencias externas y se puede compilar con un único comando.
    ```sh
    cc -o monopoly monopoly.c
    ```
4.  **Ejecuta la aplicación:**
    ```sh
    ./monopoly
    ```
    Aparecerá el menú principal y podrás iniciar una nueva partida, cargar una anterior o consultar las reglas.

## Licencia

Distribuido bajo la Licencia MIT. Consulta `LICENSE` para más información.

## Contacto

Proyecto realizado por **Alberto ESPERÓN**.

-   Alberto ESPERÓN - [Perfil de LinkedIn](https://www.linkedin.com/in/alberto-espfer) - [Perfil de GitHub](https://github.com/Alespfer)
