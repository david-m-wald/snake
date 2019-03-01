# Snake

#### Console Snake game written in C

<p align="center"><img src="https://imgur.com/fphWffN.gif"  width=700></p>

## Language

- C

## Features

- Automatic movement of snake with direction shifting by pressing arrow keys
- Color-coded head (red) and body/tail (green) segments to distinguish active position on screen
- Snake segment position tracking using a queue
- Randomly positioned powerups for growth (currently single-segment growth -- updateable in future)
- Constant gameplay speed (updateable in future)
- Displays current score and session hi-score

## Installation / To Play

Run **Snake.exe** file included with the latest release

## Usage / Gameplay Instructions

The goal of the game is to maneuver the snake around the game board, collecting powerups to grow as long as possible without it running into a boundary or itself.

When starting a new gameplay session, press any key to begin a game.

A new game starts with a single-segment snake positioned at the center of the game board. The snake moves automatically at a fixed speed, one space at a time, and initially to the right on screen. Use the arrow keys on the keyboard to change direction (see more below) and navigate toward the current powerup, symbolized by an 'x' on screen. Collecting a powerup will cause the snake to grow in length by a single segment. Each time a powerup is collected, a new powerup will be randomly placed on the game board on a space not currently occupied by the snake. The head of the snake, the position for which the player controls, is colored in red while the other segments of the snake are colored in green.

The snake is allowed to continue moving in its current direction or move in a new direction except for backward on itself when it is longer than one segment. When the snake consists of only one segment (i.e., the head), it can freely move in all four cardinal directions (up, down, left, and right) or, in other words, the current direction and three new directions. When the snake is longer than one segment, it can move in only three directions (the previous direction and new directions to either side). For example, a multi-segment snake currently moving up can only move up, left, or right, but not down.

In general, the direction in which the snake moves is changed by pressing a valid arrow key between successive moves. Prior to the first move of the game, a valid arrow key can be pressed to change the default starting direction (right). Only those arrow keys that will cause one of the aforementioned, permitted direction changes is considered valid. As one example, the left, right, and down arrow keys are valid for a single-segment snake currently moving up. As another example, only the left and right arrow keys are valid for a multi-segment snake currently moving up. Ultimately, only the first valid arrow key pressed between successive moves will trigger a direction shift; subsequent arrow keys pressed before the next move is completed are ignored. Pressing an invalid arrow key or other key has no effect on gameplay. If no valid arrow key is pressed between successive moves, the snake will maintain its current direction.

The current game score is equal to the length (i.e., number of segments) of the snake. The current score and session hi-score are provided above the game board.

The game is over when the next move results in the snake crashing into a boundary or itself. The player will then be prompted to play a new game or close the application.

## Potential Future Work

- Variable gameplay speeds
- Multi-segment growth powerups
- Obstacles
- Variable game board sizes
- Saved hi-score between gameplay sessions

## Version History

#### v1.0.0 -- February 4, 2019

- Initial release
- Single-size game board
- Constant speed
- Single-segment growth powerups

#### v1.0.1 -- February 28, 2019

- Fixed window centering