# Gang-Wars-Competition
This is an AI Agent that uses Minimax algorithm and Alpha-beta pruning with adaptive depth control in order to play against a human or AI opponent in a time-based special board game. It ranked 5th in the Gang Wars competition that ran the program against 61 other competing programs.

##Input Format: Input.txt
```
N: (int) Size of game board {Has to be within 1 and 26}
Mode: (string) Mode of operation for the agent
    Options: 
        MINIMAX :  Used to test underlying MINIMAX algorithm
        ALPHABETA : Used to test underlying ALPHABETA algorithm
        COMPETITION : Used to run agent in competition mode which uses a highly 
                      optimized version of the alphabeta algorithm
YouPlay: (char) The character that is used to refresent your agent on the game board
    Options: X | O
TimeLimit: (float) The maximum time agent has to complete (win/draw/lose) in the game
    {Is used as depth (int) in MINIMAX and ALPHABETA mode}
BoardValues: (int[N][N]) The value for each place on the board
BoardState: (char[N][N]) The state of the board represented using the following notation
    X: X owns the current cell of the board
    O: O owns the current cell of the board
    .: The current cell is not owned by any body
```

##Output Format: Output.txt
```
BestMove: (string) A string of length 2 indicating the best cell to capture
MoveType: (string) A string indicating what type of move is the best
    Options:
        Stake: Simply capture the cell
        Raid: Capture the cell along with all neighbouring enemy cells
BoardState: (char[N][N]) Shows the state of the board after the move is played
```

##Examples

```
5
COMPETITION
X
10
20 16 1 32 30
20 12 2 11 8
28 48 9 1 1
20 12 10 6 2
25 30 23 21 10
..XX.
..XOX
...O.
..OO.
.....
```
```
3
MINIMAX
O
2
1 8 23
5 42 12
26 30 9
X..
...
...
```
```
5
ALPHABETA
X
4
20 16 1 32 30
20 12 2 11 8
28 48 9 1 1
20 12 10 6 2
25 30 23 21 10
..XX.
..XOX
...O.
..OO.
.....
```


If needed you can write a simple server program that pits the agent against itself or a human player.
