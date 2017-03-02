#Input Format:
N: (int) Size of game board
Mode: (string) Mode of operation for the agent
    Options: 
        MINIMAX :  Used to test underlying MINIMAX algorithm
        ALPHABETA : Used to test underlying ALPHABETA algorithm
        COMPETITION : Used to run agent in competition mode which uses a highly optimized version of the alphabeta algorithm
YouPlay: (char) The character that is used to refresent your agent on the game board
    Options: X | O
TimeLimit: (float) The maximum time agent has to complete (win/draw/lose) in the game
    {Is used as depth (int) in MINIMAX and ALPHABETA mode}
BoardValues: (int[N][N]) The value for each place on the board
BoardState: (char[N][N]) The state of the board represented using the following notation
    X: X owns the current cell of the board
    O: O owns the current cell of the board
    .: The current cell is not owned by any body



3
COMPETITION
O
2
1 8 23
5 42 12
26 30 9
X..
...
...


5
MINIMAX
X
1
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



6
COMPETITION
X
1
1 1 1 1 1 1
1 1 1 1 1 1
1 1 1 1 1 1
1 1 1 1 1 1
1 1 1 1 1 1
1 1 1 1 1 1
......
......
......
......
......
......



6
ALPHABETA
O
1
1 1 1 1 1 1
1 1 1 1 1 1
1 1 1 1 1 1
1 1 1 1 1 70
1 1 1 1 1 1
1 1 1 1 1 1
......
......
......
......
......
......



6
ALPHABETA
O
1
1 1 1 1 1 1
1 1 1 1 1 1
1 1 1 1 1 1
1 1 1 1 1 1
1 1 1 1 1 1
1 1 1 1 1 2
...X.O
......
......
......
......
......




6
ALPHABETA
O
1
1 1 1 1 1 1
1 1 1 1 1 1
1 1 1 1 1 1
1 1 1 1 1 1
1 1 1 1 1 1
1 1 1 1 1 3
...X.O
......
......
......
......
......




4
ALPHABETA
X
6
4 1 2 4
2 3 1 2
4 3 3 4
3 3 3 2
...X
X..X
O..O
...O



3
MINIMAX
X
4
10 20 30
40 50 60
70 80 90
X..
.O.
...
