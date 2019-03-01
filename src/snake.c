#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define GAME_WIDTH 80	//Full width including boundaries
#define GAME_HEIGHT 20	//Full height including boundaries
#define LEFT_MARGIN 0	//Margin for game space
#define TOP_MARGIN 1	//Margin for game space

HANDLE hStdout, hStdin;

typedef struct Segment {
	COORD location;
	struct Segment *nextSegment;
} Segment, *SegmentPtr;

void emptyQueue(SegmentPtr frontPtr);
void addHead(SegmentPtr* lastPtr, COORD position);
void removeTail(SegmentPtr *firstPtr);
void showNewHead(COORD position);
void hideOldTail(COORD position);
void changeOldHead(COORD position);
void move(SegmentPtr* firstPtr, SegmentPtr* lastPtr, COORD position, boolean grow);
void printBoundary(int firstRow, int lastRow, int firstCol, int lastCol);
void printScoreboard(COORD scoreTitlePos, COORD hiScoreTitlePos);
void printScore(int score, COORD scorePos);
void generatePowerup(int firstRow, int lastRow, int firstCol, int lastCol);
boolean isGameOver(COORD headPosition, COORD oldTailPosition);
boolean isPowerup(COORD position);

int main(void)
{
	//Game display properties
	HWND consoleWindow, desktopScreen;
	RECT oldConsoleWindowRect, desktopScreenRect;
	int desktopScreenWidth, desktopScreenHeight, consoleWindowWidth, consoleWindowHeight;
	int newConsoleWindowPosX, newConsoleWindowPosY;
	LONG_PTR oldConsoleWindowStyle, newConsoleWindowStyle;
	COORD screenBSize = { GAME_WIDTH + LEFT_MARGIN, GAME_HEIGHT + TOP_MARGIN };
	int leftBound = LEFT_MARGIN;
	int rightBound = screenBSize.X - 1;
	int topBound = TOP_MARGIN;
	int bottomBound = screenBSize.Y - 1;
	SMALL_RECT windowSize = { 0, 0, rightBound, bottomBound };
	CONSOLE_FONT_INFOEX oldFontInfo, newFontInfo;
	COORD fontSize = { 0, 16 };
	int fontWeight = 1000;
	CONSOLE_CURSOR_INFO cursor = { 25, FALSE };
		
	//Queue and positioning variables
	SegmentPtr headPtr = NULL;
	SegmentPtr tailPtr = NULL;
	COORD headPos, lastMove;

	//Input event information
	INPUT_RECORD events[100], event;
	KEY_EVENT_RECORD keyEvent;
	KEY_EVENT_RECORD emptyKeyEvent = { FALSE, 0, 0, 0, { 0 }, 0 };
	INPUT_RECORD emptyEvent = { KEY_EVENT, emptyKeyEvent };
	DWORD nWriteEvents, nReadEvents;
	boolean arrowKey;

	//Other setup variables
	boolean newGame = TRUE;
	int speed = 150; //ms for delay
	int score, hiScore = 1;
	COORD scoreTitlePos = { LEFT_MARGIN, TOP_MARGIN - 1 };
	COORD hiScoreTitlePos = { LEFT_MARGIN + 16, TOP_MARGIN - 1};
	COORD scorePos = { scoreTitlePos.X + 7, scoreTitlePos.Y };
	COORD hiScorePos = { hiScoreTitlePos.X + 10, scoreTitlePos.Y };
	
	//Game setup
	hStdin = GetStdHandle(STD_INPUT_HANDLE);
	hStdout = GetStdHandle(STD_OUTPUT_HANDLE);

	SetConsoleWindowInfo(hStdout, TRUE, &windowSize);
	SetConsoleScreenBufferSize(hStdout, screenBSize);

	oldFontInfo.cbSize = sizeof(CONSOLE_FONT_INFOEX);
	GetCurrentConsoleFontEx(hStdout, FALSE, &oldFontInfo);
	newFontInfo = oldFontInfo;
	newFontInfo.dwFontSize = fontSize;
	newFontInfo.FontWeight = fontWeight;
	SetCurrentConsoleFontEx(hStdout, FALSE, &newFontInfo); //set font here to ensure proper window sizing before positioning
	SetConsoleCursorInfo(hStdout, &cursor);
	SetConsoleTitle("Snake");

	consoleWindow = GetConsoleWindow();
	desktopScreen = GetDesktopWindow();
	GetWindowRect(consoleWindow, &oldConsoleWindowRect);
	GetWindowRect(desktopScreen, &desktopScreenRect);
	desktopScreenWidth = desktopScreenRect.right - desktopScreenRect.left;
	desktopScreenHeight = desktopScreenRect.bottom - desktopScreenRect.top;
	consoleWindowWidth = oldConsoleWindowRect.right - oldConsoleWindowRect.left;
	consoleWindowHeight = oldConsoleWindowRect.bottom - oldConsoleWindowRect.top;
	newConsoleWindowPosX = desktopScreenWidth / 2 - consoleWindowWidth / 2;	  //center window on screen
	newConsoleWindowPosY = desktopScreenHeight / 2 - consoleWindowHeight / 2; //center window on screen
	SetWindowPos(consoleWindow, HWND_TOP, newConsoleWindowPosX, newConsoleWindowPosY, 0, 0, SWP_NOSIZE);

	oldConsoleWindowStyle = GetWindowLongPtr(consoleWindow, GWL_STYLE);
	newConsoleWindowStyle = oldConsoleWindowStyle ^ (WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_SIZEBOX); //prevent resizing
	SetWindowLongPtr(consoleWindow, GWL_STYLE, newConsoleWindowStyle);
	
	srand((unsigned int)time(NULL));

	printf("Snake\n\n");
	printf("Use the arrow keys to change direction.\n\n");
	printf("Press any key to start!");
	while (1) {
		//Read events and process first pressed key to start game
		ReadConsoleInput(hStdin, events, 1, &nReadEvents);
		if (events[0].EventType == KEY_EVENT) break;
	}

	while (newGame) {
		//Clear screen, print boundary, and print score information
		system("cls");
		printBoundary(topBound, bottomBound, leftBound, rightBound);
		printScoreboard(scoreTitlePos, hiScoreTitlePos);
		score = 1;
		printScore(score, scorePos);
		printScore(hiScore, hiScorePos);
				
		//Set starting position and initialize first move to right
		headPos.X = (leftBound + rightBound) / 2;
		headPos.Y = (topBound + bottomBound) / 2;
		lastMove.X = 1;
		lastMove.Y = 0;

		//Initial placement of head and generation of first powerup
		emptyQueue(tailPtr);
		headPtr = NULL;
		move(&tailPtr, &headPtr, headPos, TRUE);
		tailPtr = headPtr;
		generatePowerup(topBound + 1, bottomBound - 1, leftBound + 1, rightBound - 1);

		//Gameplay
		while (1) {
			FlushConsoleInputBuffer(hStdin);

			//Add empty event to input buffer to read if no input events are generated during delay - guarantees automatic movement
			WriteConsoleInput(hStdin, &emptyEvent, 1, &nWriteEvents);

			arrowKey = FALSE;
			Sleep(speed); //Delay to simulate speed and allow for input events

			//Read events and process first pressed arrow key in new direction (score == 1) or adjacent direction (score > 1)
			ReadConsoleInput(hStdin, events, 100, &nReadEvents);
			for (int i = 1; i < (int)nReadEvents; i++) { //i = 1 since first event is the empty event
				event = events[i];
				if (event.EventType == KEY_EVENT) {
					keyEvent = event.Event.KeyEvent;
					if (keyEvent.bKeyDown) {
						switch (keyEvent.wVirtualKeyCode) {
							case VK_RIGHT:
								if (!(lastMove.X == 1 || (lastMove.X == -1 && score > 1))) {
									headPos.X++;
									lastMove.X = 1;
									lastMove.Y = 0;
									arrowKey = TRUE;
								}
								break;
							case VK_LEFT:
								if (!(lastMove.X == -1 || (lastMove.X == 1 && score > 1))) {
									headPos.X--;
									lastMove.X = -1;
									lastMove.Y = 0;
									arrowKey = TRUE;
								}
								break;
							case VK_UP:
								if (!(lastMove.Y == -1 || (lastMove.Y == 1 && score > 1))) {
									headPos.Y--;
									lastMove.X = 0;
									lastMove.Y = -1;
									arrowKey = TRUE;
								}
								break;
							case VK_DOWN:
								if (!(lastMove.Y == 1 || (lastMove.Y == -1 && score > 1))) {
									headPos.Y++;
									lastMove.X = 0;
									lastMove.Y = 1;
									arrowKey = TRUE;
								}
								break;
							default: //non-arrow key or arrow key not accepted
								break;
						}

						if (arrowKey) break;
					}
				}
			}

			//If accepted arrow key not pressed in time, move in same direction as previous
			if (!arrowKey) {
				headPos.X += lastMove.X;
				headPos.Y += lastMove.Y;
			}

			//Game Over? - display last move and prompt for replay
			if (isGameOver(headPos, tailPtr->location)) {
				move(&tailPtr, &headPtr, headPos, FALSE);
				newGame = MessageBox(consoleWindow, "Game Over.\n Replay?", "Game Over.", MB_YESNO | MB_ICONQUESTION ) == IDYES ? TRUE : FALSE;
				break;
			}
			
			//Powerup?
			if (isPowerup(headPos)) {
				move(&tailPtr, &headPtr, headPos, TRUE);
				score++;
				printScore(score, scorePos);
				if (score > hiScore) {
					hiScore++;
					printScore(hiScore, hiScorePos);
				}
				generatePowerup(topBound + 1, bottomBound - 1, leftBound + 1, rightBound - 1);
			}
			else
				move(&tailPtr, &headPtr, headPos, FALSE);
		}
	}

	return 0;
}

//Function empties queue
void emptyQueue(SegmentPtr frontPtr) {
	SegmentPtr currentPtr, nextPtr;

	currentPtr = frontPtr;
	while (currentPtr != NULL) { 
		nextPtr = currentPtr->nextSegment;
		free(currentPtr);
		currentPtr = nextPtr;
	}
}

//Function adds new head segment to back of queue
void addHead(SegmentPtr* backPtr, COORD position) {
	SegmentPtr newPtr = malloc(sizeof(Segment));

	if (newPtr == NULL)
		;
	else {
		if (*backPtr != NULL) (*backPtr)->nextSegment = newPtr;
		*backPtr = newPtr;
		newPtr->location = position;
		newPtr->nextSegment = NULL;
	}
}

//Function removes tail segment from front of queue
void removeTail(SegmentPtr *frontPtr) {
	SegmentPtr tempPtr = *frontPtr;
	(*frontPtr) = (*frontPtr)->nextSegment;
	free(tempPtr);
}

//Function changes appearance of old head to that of regular segment
void changeOldHead(COORD position) {
	DWORD nWriteChar;
	WORD attributes = FOREGROUND_GREEN | FOREGROUND_INTENSITY;

	WriteConsoleOutputCharacter(hStdout, "O", 1, position, &nWriteChar);
	WriteConsoleOutputAttribute(hStdout, &attributes, 1, position, &nWriteChar);
}

//Function displays new head on screen
void showNewHead(COORD position) {
	DWORD nWriteChar;
	DWORD nReadChar;
	TCHAR character;
	WORD attributes = FOREGROUND_RED | FOREGROUND_INTENSITY;
	
	ReadConsoleOutputCharacter(hStdout, &character, 1, position, &nReadChar);
	if (character == '#') //head collides with boundary
		attributes = 252; //white background with red letters
	WriteConsoleOutputCharacter(hStdout, "O", 1, position, &nWriteChar);
	WriteConsoleOutputAttribute(hStdout, &attributes, 1, position, &nWriteChar);
}

//Function hides old tail from screen
void hideOldTail(COORD position) {
	DWORD nWriteChar;
	WORD attributes = 0; //black background with black letters

	WriteConsoleOutputCharacter(hStdout, "", 1, position, &nWriteChar);
	WriteConsoleOutputAttribute(hStdout, &attributes, 1, position, &nWriteChar);
}

//Function delegates all move and display operations
void move(SegmentPtr *frontPtr, SegmentPtr *backPtr, COORD position, boolean grow) {
	//change display for old head segment if queue has been created and snake > 1 segment and/or will grow
	if (*backPtr != NULL && (*frontPtr != *backPtr || grow))
		changeOldHead((*backPtr)->location);
	
	addHead(backPtr, position);
	if (!grow) {
		hideOldTail((*frontPtr)->location);
		removeTail(frontPtr);
	}
	showNewHead(position); //show head after hiding tail if head moves to space occupied by tail just prior
}

//Function prints boundary of game space
void printBoundary(int firstRow, int lastRow, int firstCol, int lastCol) {
	COORD position;
	DWORD nWriteChar;
	WORD attributes = 255; //white letters on white background

	for (int row = firstRow; row <= lastRow; row++) {
		for (int col = firstCol; col <= lastCol; col++) {
			if (row == firstRow || row == lastRow || col == firstCol || col == lastCol) {
				position.X = col;
				position.Y = row;
				WriteConsoleOutputCharacter(hStdout, "#", 1, position, &nWriteChar);
				WriteConsoleOutputAttribute(hStdout, &attributes, 1, position, &nWriteChar);
			}
		}
	}
}

//Function displays scoreboard
void printScoreboard(COORD scoreTitlePos, COORD hiScoreTitlePos) {
	DWORD nWriteChar;

	WriteConsoleOutputCharacter(hStdout, "SCORE: ", 7, scoreTitlePos, &nWriteChar);
	WriteConsoleOutputCharacter(hStdout, "HI-SCORE: ", 10, hiScoreTitlePos, &nWriteChar);
}

//Function displays scores
void printScore(int score, COORD scorePos) {
	DWORD nWriteChar;
	TCHAR strScore[5];

	_itoa_s(score, strScore, 5, 10);
	WriteConsoleOutputCharacter(hStdout, strScore, strlen(strScore), scorePos, &nWriteChar);
}

//Function adds growth powerup to screen at random location
void generatePowerup(int firstRow, int lastRow, int firstCol, int lastCol) {
	TCHAR character;
	DWORD nReadChar, nWriteChar;
	COORD position;
	WORD attributes = 15; //white letters on black background

	while (1) {
		position.Y = firstRow + rand() % (lastRow - firstRow + 1);
		position.X = firstCol + rand() % (lastCol - firstCol + 1);
		ReadConsoleOutputCharacter(hStdout, &character, 1, position, &nReadChar);
		if (character != 'O') break; //random number only valid for empty space
	}

	WriteConsoleOutputCharacter(hStdout, "x", 1, position, &nWriteChar);
	WriteConsoleOutputAttribute(hStdout, &attributes, 1, position, &nWriteChar);
}

//Function checks if head runs into boundary or snake body except for old tail position
boolean isGameOver(COORD headPosition, COORD oldTailPosition) {
	TCHAR character;
	DWORD nReadChar;

	ReadConsoleOutputCharacter(hStdout, &character, 1, headPosition, &nReadChar);
	if (headPosition.X == oldTailPosition.X && headPosition.Y == oldTailPosition.Y) return FALSE;
	return character == '#' || character == 'O';
}

//Function checks if head runs into powerup
boolean isPowerup(COORD position) {
	TCHAR character;
	DWORD nReadChar;

	ReadConsoleOutputCharacter(hStdout, &character, 1, position, &nReadChar);
	return character == 'x';
}