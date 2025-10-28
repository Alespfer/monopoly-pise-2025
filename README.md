<div align="right">
  <a href="./README.md">English</a> |
  <a href="./README_fr.md">Français</a> |
  <a href="./README_es.md">Español</a>
</div>

# Console Monopoly in C

![License](https://img.shields.io/badge/license-MIT-blue.svg)
![Language](https://img.shields.io/badge/language-C-green.svg)
![Platform](https://img.shields.io/badge/platform-Linux%20%7C%20macOS-lightgrey.svg)

A complete and functional text-based simulation of the classic Monopoly board game, developed in C. This project implements all the official rules, including property management (buying, building, mortgaging), auctions, player negotiations, and multiple game modes, all within a Unix-like terminal.

![Main Game Board](./screenshots/screenshot-board.png)

## Table of Contents

- [About The Project](#about-the-project)
- [Key Features](#key-features)
- [Architecture & Design](#architecture--design)
- [Technical Stack](#technical-stack)
- [Getting Started](#getting-started)
- [License](#license)
- [Contact](#contact)

## About The Project

This project was developed as part of a university curriculum at Université Paris Cité. The main objective was to implement a complex game logic engine from scratch in C, without external dependencies, focusing on robust data structures, state management, and user interaction in a console environment.

## Key Features

-   🎲 **Complete Game Engine:** Manages player turns, dice rolls, movement, prison rules, and card draws ("Chance" & "Community Chest").
-   🏛️ **Advanced Property Management:** Buy properties, build houses and hotels (respecting the balanced-build rule), collect rent, and mortgage assets to raise funds.
-   🤝 **Rich Player Interaction:** Features an automated auction system when a player declines to buy a property, and a comprehensive negotiation module for trading properties, cash, and "Get Out of Jail Free" cards.
-   🏆 **Three Distinct Game Modes:**
    1.  **Classic:** The last player standing after all others have gone bankrupt wins.
    2.  **Limited Turns:** The wealthiest player wins after a predetermined number of turns.
    3.  **Monetary Goal:** The first player to reach a target net worth wins.
-   🎨 **Elaborate ASCII Interface:** A full-color, dynamically updated game board rendered in ASCII, showing property groups, buildings, and player token positions in real-time.
-   💾 **Game Persistence:** Save the complete state of any ongoing game to a file and load it later to resume playing.
-   📜 **High Score Leaderboard:** The top 10 scores are saved and displayed, encouraging replayability.

## Architecture & Design

The application is built as a **monolithic C program** (all source code resides in a single `monopoly.c` file). The game state is managed through a set of global variables and `struct` data structures (`Joueur`, `CasePlateau`, `Carte`), which is a pragmatic approach for simplifying information flow between functions in this context.

Data persistence is handled via a **custom text-based serialization mechanism**. Game saves and high scores are written to portable `.txt` files, allowing a game session to be easily moved and resumed on another compatible machine.

## Technical Stack

-   **Language:** C (C11 Standard)
-   **Compiler:** GCC / Clang
-   **Environment:** Unix-like Terminal (Linux, macOS)
-   **Dependencies:** None (only standard C libraries).

## Getting Started

To get a local copy up and running, follow these simple steps.

1.  **Clone the repository:**
    ```sh
    git clone https://github.com/Alespfer/monopoly-pise-2025.git
    ```
2.  **Navigate to the project directory:**
    ```sh
    cd monopoly-pise-2025
    ```
3.  **Compile the program:**
    The project has no external dependencies and can be compiled with a single command.
    ```sh
    cc -o monopoly monopoly.c
    ```
4.  **Run the application:**
    ```sh
    ./monopoly
    ```
    The main menu will appear, and you can start a new game, load a previous one, or check the rules.

## License

Distributed under the MIT License. See `LICENSE` for more information.

## Contact

Project by **Alberto ESPERÓN**.

-   Alberto ESPERÓN - [LinkedIn](https://www.linkedin.com/in/alberto-espfer) - [GitHub](https://github.com/Alespfer)
