//  Windows
#ifdef _WIN32

#include <Windows.h>

double get_wall_time() {
	LARGE_INTEGER time, freq;
	if (!QueryPerformanceFrequency(&freq)) {
		//  Handle error
		return 0;
	}
	if (!QueryPerformanceCounter(&time)) {
		//  Handle error
		return 0;
	}
	return (double) time.QuadPart / freq.QuadPart;
}

double get_cpu_time() {
	FILETIME a, b, c, d;
	if (GetProcessTimes(GetCurrentProcess(), &a, &b, &c, &d) != 0) {
		//  Returns total user time.
		//  Can be tweaked to include kernel times as well.
		return (double) (d.dwLowDateTime | ((unsigned long long) d.dwHighDateTime << 32)) * 0.0000001;
	} else {
		//  Handle error
		return 0;
	}
}

//  Posix/Linux
#else
#include <time.h>
#include <sys/time.h>
double get_wall_time(){
	struct timeval time;
	if (gettimeofday(&time,NULL)){
		//  Handle error
		return 0;
	}
	return (double)time.tv_sec + (double)time.tv_usec * .000001;
}
double get_cpu_time(){
	return (double)clock() / CLOCKS_PER_SEC;
}
#endif

#include <iostream>
#include <fstream>
#include <sstream>
#include <math.h>

#define INF 2147483647

using namespace std;

double TimePerNode = 0;

double getRuntime(int BranchingFactor, int Depth) {
	return ((pow(BranchingFactor, Depth + 1) / (BranchingFactor - 1)) * TimePerNode);
}

int getDepth(int BranchingFactor, double Time) {
	if (BranchingFactor == 1)
		return 1;

	//Calculate estimated time for making the current move
	double ExecutionTimeRation = Time / (int) round(BranchingFactor / 2.0);
	cout << "Time Ration : " << ExecutionTimeRation << " seconds" << endl;

	//Get time required to process single node
	ifstream CurrentDataFile("GameData.txt");
	if (CurrentDataFile.is_open()) {
		string TempString;
		getline(CurrentDataFile, TempString);
		TimePerNode = stod(TempString);
		CurrentDataFile.close();
	} else {
		ifstream OriginalDataFile("CPU.txt");
		if (OriginalDataFile.is_open()) {
			//Get CPU Time per node
			string TempString;
			getline(OriginalDataFile, TempString);
			TimePerNode = stod(TempString);
			OriginalDataFile.close();
		}
	}

	//Calculate the maximum depth that can be traversed in the allocated time
	double Depth = (double) (
			(log(((ExecutionTimeRation / TimePerNode) * (BranchingFactor - 1)) + 1) / log(BranchingFactor)) - 1);

	if (Depth < 0)
		return 1;

	//Depth Should be lesser than or equal to Branching Factor
	if (Depth >= BranchingFactor)
		return BranchingFactor;

	cout << "Floating Depth : " << Depth << endl;

	//If possible try going deeper unless within 5 seconds of remaining total game time
	if (((Depth - (int) Depth) > 0.5) && (getRuntime(BranchingFactor, (int) Depth) + 5 < Time))
		return ((int) Depth + 1);
	else
		return ((int) Depth);
}

void updateTimePerNode(double RunTime, int BranchingFactor, int Depth) {
	if (BranchingFactor == 0)
		return;

	ofstream OutputFile("GameData.txt");
	if (OutputFile.is_open()) {
		double NewTimePerNode = RunTime / ((pow(BranchingFactor, Depth + 1) - 1) / (BranchingFactor - 1));
		if (TimePerNode > NewTimePerNode)
			OutputFile << (TimePerNode + NewTimePerNode) / 2;
		else
			OutputFile << NewTimePerNode;

		OutputFile.close();
	}
}

class MiniMax {
private:
	int **BoardValue;
	char YouPlay;
	int BoardSize;

	struct move {
		int Row, Column, Type; //Stake = 0, Raid = 1
	};

	struct node {
		char **BoardState;
		move Move, BestNextMove;
		int Score, AvailableMoves;
		char Turn;              //X or O and is identifies who will play the next ply
	};

	string print(node Node) {

		string output = "";

		//Print Move;
		output += (char) (65 + Node.BestNextMove.Column);
		output += to_string(1 + Node.BestNextMove.Row);
		output += " ";
		//Print MoveType
		if (Node.BestNextMove.Type == 0)
			output += "Stake";
		else
			output += "Raid";
		output += "\n";
		//Create Next Board State
		Node.BoardState[Node.BestNextMove.Row][Node.BestNextMove.Column] = Node.Turn;
		if (Node.BestNextMove.Type == 1) {
			if (Node.BestNextMove.Row > 0 &&
			    Node.BoardState[Node.BestNextMove.Row - 1][Node.BestNextMove.Column] != '.' &&
			    Node.BoardState[Node.BestNextMove.Row - 1][Node.BestNextMove.Column] != Node.Turn) {

				Node.BoardState[Node.BestNextMove.Row - 1][Node.BestNextMove.Column] = Node.Turn;
			}
			if (Node.BestNextMove.Row < BoardSize - 1 &&
			    Node.BoardState[Node.BestNextMove.Row + 1][Node.BestNextMove.Column] != '.' &&
			    Node.BoardState[Node.BestNextMove.Row + 1][Node.BestNextMove.Column] != Node.Turn) {

				Node.BoardState[Node.BestNextMove.Row + 1][Node.BestNextMove.Column] = Node.Turn;
			}
			if (Node.BestNextMove.Column > 0 &&
			    Node.BoardState[Node.BestNextMove.Row][Node.BestNextMove.Column - 1] != '.' &&
			    Node.BoardState[Node.BestNextMove.Row][Node.BestNextMove.Column - 1] != Node.Turn) {

				Node.BoardState[Node.BestNextMove.Row][Node.BestNextMove.Column - 1] = Node.Turn;
			}
			if (Node.BestNextMove.Column < BoardSize - 1 &&
			    Node.BoardState[Node.BestNextMove.Row][Node.BestNextMove.Column + 1] != '.' &&
			    Node.BoardState[Node.BestNextMove.Row][Node.BestNextMove.Column + 1] != Node.Turn) {

				Node.BoardState[Node.BestNextMove.Row][Node.BestNextMove.Column + 1] = Node.Turn;
			}
		}
		//Print BoardState
		for (int i = 0; i < BoardSize; i++) {
			for (int j = 0; j < BoardSize; j++)
				output += Node.BoardState[i][j];
			output += "\n";
		}

		return output;
	}

	node step(node CurrentNode, int Depth) {
		/**
		 * Terminal Test
		 * If we have explored at max depth or if we have no available moves then evaluate board and return
		 **/
		if (Depth == 0 || CurrentNode.AvailableMoves == 0) {
			//Evaluate board and return score;
			//cout << "Score : " << CurrentNode.Score << endl;
			return CurrentNode;
		}


		int MinScore = INF, MaxScore = -INF;
		node Result;

		//Stake moves
		for (int i = 0; i < BoardSize; i++) {
			for (int j = 0; j < BoardSize; j++) {
				if (CurrentNode.BoardState[i][j] == '.') {
					//Create a moveDS
					node NewNode;
					NewNode.BoardState = CurrentNode.BoardState;
					NewNode.BoardState[i][j] = CurrentNode.Turn;
					NewNode.Move.Row = i;
					NewNode.Move.Column = j;
					NewNode.Move.Type = 0; // Stake
					NewNode.Turn = CurrentNode.Turn == 'X' ? 'O' : 'X';
					NewNode.AvailableMoves = CurrentNode.AvailableMoves - 1;
					if (CurrentNode.Turn == YouPlay)
						NewNode.Score = CurrentNode.Score + BoardValue[i][j];
					else
						NewNode.Score = CurrentNode.Score - BoardValue[i][j];

					//Play the moveDS
					NewNode = step(NewNode, Depth - 1);

					if (CurrentNode.Turn == YouPlay && NewNode.Score > MaxScore) {
						/**
						 * Max Step
						 */

						MaxScore = NewNode.Score;

						//Store new best node
						Result = NewNode;
					} else if (CurrentNode.Turn != YouPlay && NewNode.Score < MinScore) {
						/**
						 * Min Step
						 */

						MinScore = NewNode.Score;

						//Store new best node
						Result = NewNode;
					}

					//Reverse moveDS from board for future updates
					CurrentNode.BoardState[i][j] = '.';

				}
			}
		}

		//Raid moves
		for (int i = 0; i < BoardSize; i++) {
			for (int j = 0; j < BoardSize; j++) {
				/**
				 * If neither player occupies the place [i][j] on the board AND
				 * The current player occupies a place above OR
				 *                                       below OR
				 *                                       left OR
				 *                                       right of the place [i][j] on the board
				 **/
				if ((CurrentNode.BoardState[i][j] == '.') &&
				    ((i > 0 && CurrentNode.BoardState[i - 1][j] == CurrentNode.Turn) ||
				     (i < BoardSize - 1 && CurrentNode.BoardState[i + 1][j] == CurrentNode.Turn) ||
				     (j > 0 && CurrentNode.BoardState[i][j - 1] == CurrentNode.Turn) ||
				     (j < BoardSize - 1 && CurrentNode.BoardState[i][j + 1] == CurrentNode.Turn))) {

					//Create a moveDS
					node NewNode;
					NewNode.BoardState = CurrentNode.BoardState;
					NewNode.BoardState[i][j] = CurrentNode.Turn;
					NewNode.Move.Row = i;
					NewNode.Move.Column = j;
					NewNode.Move.Type = 1;                              // Raid
					NewNode.Turn = CurrentNode.Turn == 'X' ? 'O' : 'X';
					NewNode.AvailableMoves = CurrentNode.AvailableMoves - 1;

					/**
					 *  Calculate the total Value of neigboring places belonging to the other player
					 *  Capture all neighboring places belonging to the other player
					 *  and store Change list of all changes for reversal at the end
					 */
					int NeighborScore = 0,
							ChangeList_i[4], ChangeList_j[4], ChangeCount = 0;
					if (i > 0 && CurrentNode.BoardState[i - 1][j] != '.' &&
					    CurrentNode.BoardState[i - 1][j] != CurrentNode.Turn) {

						NeighborScore += BoardValue[i - 1][j];

						NewNode.BoardState[i - 1][j] = CurrentNode.Turn;
						ChangeList_i[ChangeCount] = i - 1;
						ChangeList_j[ChangeCount] = j;
						ChangeCount++;
					}
					if (i < BoardSize - 1 && CurrentNode.BoardState[i + 1][j] != '.' &&
					    CurrentNode.BoardState[i + 1][j] != CurrentNode.Turn) {

						NeighborScore += BoardValue[i + 1][j];

						NewNode.BoardState[i + 1][j] = CurrentNode.Turn;
						ChangeList_i[ChangeCount] = i + 1;
						ChangeList_j[ChangeCount] = j;
						ChangeCount++;
					}
					if (j > 0 && CurrentNode.BoardState[i][j - 1] != '.' &&
					    CurrentNode.BoardState[i][j - 1] != CurrentNode.Turn) {

						NeighborScore += BoardValue[i][j - 1];

						NewNode.BoardState[i][j - 1] = CurrentNode.Turn;
						ChangeList_i[ChangeCount] = i;
						ChangeList_j[ChangeCount] = j - 1;
						ChangeCount++;
					}
					if (j < BoardSize - 1 && CurrentNode.BoardState[i][j + 1] != '.' &&
					    CurrentNode.BoardState[i][j + 1] != CurrentNode.Turn) {

						NeighborScore += BoardValue[i][j + 1];

						NewNode.BoardState[i][j + 1] = CurrentNode.Turn;
						ChangeList_i[ChangeCount] = i;
						ChangeList_j[ChangeCount] = j + 1;
						ChangeCount++;
					}

					if (ChangeCount != 0) {

						/**
						 * NeighborScore is removed from the other players tally and added to the current player
						 * Hence the NeighborScore has to be doubled
						 */
						NeighborScore *= 2;

						if (CurrentNode.Turn == YouPlay) {
							NewNode.Score = CurrentNode.Score + BoardValue[i][j] + NeighborScore;
						} else {
							NewNode.Score = CurrentNode.Score - BoardValue[i][j] - NeighborScore;
						}

						//Play the moveDS
						NewNode = step(NewNode, Depth - 1);

						//Max Step
						if (CurrentNode.Turn == YouPlay && NewNode.Score > MaxScore) {
							/**
							 * Max Step
							 */

							MaxScore = NewNode.Score;

							//Store new best node
							Result = NewNode;
						}//Min Step
						else if (CurrentNode.Turn != YouPlay && NewNode.Score < MinScore) {
							/**
							 * Min Step
							 */

							MinScore = NewNode.Score;

							//Store new best node
							Result = NewNode;
						}
					}

					//Reverse moveDS from board for future updates
					CurrentNode.BoardState[i][j] = '.';
					for (int k = 0; k < ChangeCount; k++) {
						CurrentNode.BoardState[ChangeList_i[k]][ChangeList_j[k]] = CurrentNode.Turn == 'X' ? 'O' : 'X';
					}
				}
			}
		}


		CurrentNode.BestNextMove = Result.Move;
		CurrentNode.Score = Result.Score;

		//cout << "---" << endl;
		return CurrentNode;
	}

public:
	MiniMax(int **BoardValue, int BoardSize, char YouPlay) {
		this->BoardValue = BoardValue;
		this->BoardSize = BoardSize;
		this->YouPlay = YouPlay;
	}

	string run(char **BoardState, int Score, int AvailableMoves, int Depth) {
		node Node;
		Node.BoardState = BoardState;
		Node.Score = Score;
		Node.AvailableMoves = AvailableMoves;
		Node.Turn = YouPlay;

		return print(step(Node, Depth));
	}
};

class AlphaBeta {
private:
	int **BoardValue;
	char YouPlay;
	int BoardSize;

	struct move {
		int Row, Column, Type; //Stake = 0, Raid = 1
	};

	struct node {
		char **BoardState;
		move Move, BestNextMove;
		int Score, AvailableMoves;
		char Turn;              //X or O and is identifies who will play the next ply
	};

	string print(node Node) {

		string output = "";

		//Print Move;
		output += (char) (65 + Node.BestNextMove.Column);
		output += to_string(1 + Node.BestNextMove.Row);
		output += " ";
		//Print MoveType
		if (Node.BestNextMove.Type == 0)
			output += "Stake";
		else
			output += "Raid";
		output += "\n";
		//Create Next Board State
		Node.BoardState[Node.BestNextMove.Row][Node.BestNextMove.Column] = Node.Turn;
		if (Node.BestNextMove.Type == 1) {
			if (Node.BestNextMove.Row > 0 &&
			    Node.BoardState[Node.BestNextMove.Row - 1][Node.BestNextMove.Column] != '.' &&
			    Node.BoardState[Node.BestNextMove.Row - 1][Node.BestNextMove.Column] != Node.Turn) {

				Node.BoardState[Node.BestNextMove.Row - 1][Node.BestNextMove.Column] = Node.Turn;
			}
			if (Node.BestNextMove.Row < BoardSize - 1 &&
			    Node.BoardState[Node.BestNextMove.Row + 1][Node.BestNextMove.Column] != '.' &&
			    Node.BoardState[Node.BestNextMove.Row + 1][Node.BestNextMove.Column] != Node.Turn) {

				Node.BoardState[Node.BestNextMove.Row + 1][Node.BestNextMove.Column] = Node.Turn;
			}
			if (Node.BestNextMove.Column > 0 &&
			    Node.BoardState[Node.BestNextMove.Row][Node.BestNextMove.Column - 1] != '.' &&
			    Node.BoardState[Node.BestNextMove.Row][Node.BestNextMove.Column - 1] != Node.Turn) {

				Node.BoardState[Node.BestNextMove.Row][Node.BestNextMove.Column - 1] = Node.Turn;
			}
			if (Node.BestNextMove.Column < BoardSize - 1 &&
			    Node.BoardState[Node.BestNextMove.Row][Node.BestNextMove.Column + 1] != '.' &&
			    Node.BoardState[Node.BestNextMove.Row][Node.BestNextMove.Column + 1] != Node.Turn) {

				Node.BoardState[Node.BestNextMove.Row][Node.BestNextMove.Column + 1] = Node.Turn;
			}
		}
		//Print BoardState
		for (int i = 0; i < BoardSize; i++) {
			for (int j = 0; j < BoardSize; j++)
				output += Node.BoardState[i][j];
			output += "\n";
		}

		return output;
	}

	node step(node CurrentNode, int Depth, int Alpha, int Beta) {
		/**
		 * Terminal Test
		 * If we have explored at max depth or if we have no available moves then evaluate board and return
		 **/
		if (Depth == 0 || CurrentNode.AvailableMoves == 0) {
			//Evaluate board and return score;
			//cout << "Score : " << CurrentNode.Score << endl;
			return CurrentNode;
		}

		int MinScore = INF, MaxScore = -INF;
		node Result;

		//Stake moves
		for (int i = 0; i < BoardSize; i++) {
			for (int j = 0; j < BoardSize; j++) {
				if (CurrentNode.BoardState[i][j] == '.') {
					//Create a moveDS
					node NewNode;
					NewNode.BoardState = CurrentNode.BoardState;
					NewNode.BoardState[i][j] = CurrentNode.Turn;
					NewNode.Move.Row = i;
					NewNode.Move.Column = j;
					NewNode.Move.Type = 0; // Stake
					NewNode.Turn = CurrentNode.Turn == 'X' ? 'O' : 'X';
					NewNode.AvailableMoves = CurrentNode.AvailableMoves - 1;
					if (CurrentNode.Turn == YouPlay)
						NewNode.Score = CurrentNode.Score + BoardValue[i][j];
					else
						NewNode.Score = CurrentNode.Score - BoardValue[i][j];

					//Play the moveDS
					NewNode = step(NewNode, Depth - 1, Alpha, Beta);

					bool AlphaCut = false, BetaCut = false;

					if (CurrentNode.Turn == YouPlay && NewNode.Score > MaxScore) {
						/**
						 * Max Step
						 */

						MaxScore = NewNode.Score;

						if (MaxScore >= Beta) {
							BetaCut = true;
						}

						Alpha = max(Alpha, MaxScore);

						//Store new best node
						Result = NewNode;

					} else if (CurrentNode.Turn != YouPlay && NewNode.Score < MinScore) {
						/**
						 * Min Step
						 */

						MinScore = NewNode.Score;

						if (MinScore <= Alpha) {
							AlphaCut = true;
						}

						Beta = min(Beta, MinScore);

						//Store new best node
						Result = NewNode;

					}

					//Reverse moveDS from board for future updates
					CurrentNode.BoardState[i][j] = '.';

					//Tree pruning
					if (BetaCut || AlphaCut) {
						CurrentNode.BestNextMove = Result.Move;
						CurrentNode.Score = Result.Score;

						return CurrentNode;
					}
				}
			}
		}

		//Raid moves
		for (int i = 0; i < BoardSize; i++) {
			for (int j = 0; j < BoardSize; j++) {
				/**
				 * If neither player occupies the place [i][j] on the board AND
				 * The current player occupies a place above OR
				 *                                       below OR
				 *                                       left OR
				 *                                       right of the place [i][j] on the board
				 **/
				if ((CurrentNode.BoardState[i][j] == '.') &&
				    ((i > 0 && CurrentNode.BoardState[i - 1][j] == CurrentNode.Turn) ||
				     (i < BoardSize - 1 && CurrentNode.BoardState[i + 1][j] == CurrentNode.Turn) ||
				     (j > 0 && CurrentNode.BoardState[i][j - 1] == CurrentNode.Turn) ||
				     (j < BoardSize - 1 && CurrentNode.BoardState[i][j + 1] == CurrentNode.Turn))) {

					//Create a moveDS
					node NewNode;
					NewNode.BoardState = CurrentNode.BoardState;
					NewNode.BoardState[i][j] = CurrentNode.Turn;
					NewNode.Move.Row = i;
					NewNode.Move.Column = j;
					NewNode.Move.Type = 1;                              // Raid
					NewNode.Turn = CurrentNode.Turn == 'X' ? 'O' : 'X';
					NewNode.AvailableMoves = CurrentNode.AvailableMoves - 1;

					/**
					 *  Calculate the total Value of neigboring places belonging to the other player
					 *  Capture all neighboring places belonging to the other player
					 *  and store Change list of all changes for reversal at the end
					 */
					int NeighborScore = 0,
							ChangeList_i[4], ChangeList_j[4], ChangeCount = 0;
					if (i > 0 && CurrentNode.BoardState[i - 1][j] != '.' &&
					    CurrentNode.BoardState[i - 1][j] != CurrentNode.Turn) {

						NeighborScore += BoardValue[i - 1][j];

						NewNode.BoardState[i - 1][j] = CurrentNode.Turn;
						ChangeList_i[ChangeCount] = i - 1;
						ChangeList_j[ChangeCount] = j;
						ChangeCount++;
					}
					if (i < BoardSize - 1 && CurrentNode.BoardState[i + 1][j] != '.' &&
					    CurrentNode.BoardState[i + 1][j] != CurrentNode.Turn) {

						NeighborScore += BoardValue[i + 1][j];

						NewNode.BoardState[i + 1][j] = CurrentNode.Turn;
						ChangeList_i[ChangeCount] = i + 1;
						ChangeList_j[ChangeCount] = j;
						ChangeCount++;
					}
					if (j > 0 && CurrentNode.BoardState[i][j - 1] != '.' &&
					    CurrentNode.BoardState[i][j - 1] != CurrentNode.Turn) {

						NeighborScore += BoardValue[i][j - 1];

						NewNode.BoardState[i][j - 1] = CurrentNode.Turn;
						ChangeList_i[ChangeCount] = i;
						ChangeList_j[ChangeCount] = j - 1;
						ChangeCount++;
					}
					if (j < BoardSize - 1 && CurrentNode.BoardState[i][j + 1] != '.' &&
					    CurrentNode.BoardState[i][j + 1] != CurrentNode.Turn) {

						NeighborScore += BoardValue[i][j + 1];

						NewNode.BoardState[i][j + 1] = CurrentNode.Turn;
						ChangeList_i[ChangeCount] = i;
						ChangeList_j[ChangeCount] = j + 1;
						ChangeCount++;
					}


					bool AlphaCut = false, BetaCut = false;

					if (ChangeCount != 0) {

						/**
						 * NeighborScore is removed from the other players tally and added to the current player
						 * Hence the NeighborScore has to be doubled
						 */
						NeighborScore *= 2;

						if (CurrentNode.Turn == YouPlay) {
							NewNode.Score = CurrentNode.Score + BoardValue[i][j] + NeighborScore;
						} else {
							NewNode.Score = CurrentNode.Score - BoardValue[i][j] - NeighborScore;
						}

						//Play the moveDS
						NewNode = step(NewNode, Depth - 1, Alpha, Beta);

						//Max Step
						if (CurrentNode.Turn == YouPlay && NewNode.Score > MaxScore) {
							/**
							 * Max Step
							 */

							MaxScore = NewNode.Score;

							if (MaxScore >= Beta) {
								BetaCut = true;
							}

							Alpha = max(Alpha, MaxScore);

							//Store new best node
							Result = NewNode;

						}//Min Step
						else if (CurrentNode.Turn != YouPlay && NewNode.Score < MinScore) {
							/**
							 * Min Step
							 */

							MinScore = NewNode.Score;

							if (MinScore <= Alpha) {
								AlphaCut = true;
							}

							Beta = min(Beta, MinScore);

							//Store new best node
							Result = NewNode;

						}
					}

					//Reverse moveDS from board for future updates
					CurrentNode.BoardState[i][j] = '.';
					for (int k = 0; k < ChangeCount; k++) {
						CurrentNode.BoardState[ChangeList_i[k]][ChangeList_j[k]] = CurrentNode.Turn == 'X' ? 'O' : 'X';
					}

					//Tree pruning
					if (BetaCut || AlphaCut) {
						CurrentNode.BestNextMove = Result.Move;
						CurrentNode.Score = Result.Score;

						return CurrentNode;
					}
				}
			}
		}


		CurrentNode.BestNextMove = Result.Move;
		CurrentNode.Score = Result.Score;

		//cout << "---" << endl;
		return CurrentNode;
	}

public:
	AlphaBeta(int **BoardValue, int BoardSize, char YouPlay) {
		this->BoardValue = BoardValue;
		this->BoardSize = BoardSize;
		this->YouPlay = YouPlay;
	}

	string run(char **BoardState, int Score, int AvailableMoves, int Depth) {
		node Node;
		Node.BoardState = BoardState;
		Node.Score = Score;
		Node.AvailableMoves = AvailableMoves;
		Node.Turn = YouPlay;

		return print(step(Node, Depth, -INF, INF));
	}
};

class Competition {
private:
	int **BoardValue;
	char YouPlay;
	int BoardSize;

	struct move {
		int Row, Column, Type; //Stake = 0, Raid = 1
	};

	struct node {
		char **BoardState;
		move Move, BestNextMove;
		int Score, AvailableMoves;
		char Turn;              //X or O and is identifies who will play the next ply
	};

	string print(node Node) {

		string output = "";

		//Print Move;
		output += (char) (65 + Node.BestNextMove.Column);
		output += to_string(1 + Node.BestNextMove.Row);
		output += " ";
		//Print MoveType
		if (Node.BestNextMove.Type == 0)
			output += "Stake";
		else
			output += "Raid";
		output += "\n";
		//Create Next Board State
		Node.BoardState[Node.BestNextMove.Row][Node.BestNextMove.Column] = Node.Turn;
		if (Node.BestNextMove.Type == 1) {
			if (Node.BestNextMove.Row > 0 &&
			    Node.BoardState[Node.BestNextMove.Row - 1][Node.BestNextMove.Column] != '.' &&
			    Node.BoardState[Node.BestNextMove.Row - 1][Node.BestNextMove.Column] != Node.Turn) {

				Node.BoardState[Node.BestNextMove.Row - 1][Node.BestNextMove.Column] = Node.Turn;
			}
			if (Node.BestNextMove.Row < BoardSize - 1 &&
			    Node.BoardState[Node.BestNextMove.Row + 1][Node.BestNextMove.Column] != '.' &&
			    Node.BoardState[Node.BestNextMove.Row + 1][Node.BestNextMove.Column] != Node.Turn) {

				Node.BoardState[Node.BestNextMove.Row + 1][Node.BestNextMove.Column] = Node.Turn;
			}
			if (Node.BestNextMove.Column > 0 &&
			    Node.BoardState[Node.BestNextMove.Row][Node.BestNextMove.Column - 1] != '.' &&
			    Node.BoardState[Node.BestNextMove.Row][Node.BestNextMove.Column - 1] != Node.Turn) {

				Node.BoardState[Node.BestNextMove.Row][Node.BestNextMove.Column - 1] = Node.Turn;
			}
			if (Node.BestNextMove.Column < BoardSize - 1 &&
			    Node.BoardState[Node.BestNextMove.Row][Node.BestNextMove.Column + 1] != '.' &&
			    Node.BoardState[Node.BestNextMove.Row][Node.BestNextMove.Column + 1] != Node.Turn) {

				Node.BoardState[Node.BestNextMove.Row][Node.BestNextMove.Column + 1] = Node.Turn;
			}
		}
		//Print BoardState
		for (int i = 0; i < BoardSize; i++) {
			for (int j = 0; j < BoardSize; j++)
				output += Node.BoardState[i][j];
			output += "\n";
		}

		return output;
	}

	node step(node CurrentNode, int Depth, int Alpha, int Beta) {
		/**
		 * Terminal Test
		 * If we have explored at max depth or if we have no available moves then evaluate board and return
		 **/
		if (Depth == 0 || CurrentNode.AvailableMoves == 0) {
			//Evaluate board and return score;
			//cout << "Score : " << CurrentNode.Score << endl;
			return CurrentNode;
		}

		int MinScore = INF, MaxScore = -INF;
		node Result;

		//Raid moves
		for (int i = 0; i < BoardSize; i++) {
			for (int j = 0; j < BoardSize; j++) {
				/**
				 * If neither player occupies the place [i][j] on the board AND
				 * The current player occupies a place above OR
				 *                                       below OR
				 *                                       left OR
				 *                                       right of the place [i][j] on the board
				 **/
				if ((CurrentNode.BoardState[i][j] == '.') &&
				    ((i > 0 && CurrentNode.BoardState[i - 1][j] == CurrentNode.Turn) ||
				     (i < BoardSize - 1 && CurrentNode.BoardState[i + 1][j] == CurrentNode.Turn) ||
				     (j > 0 && CurrentNode.BoardState[i][j - 1] == CurrentNode.Turn) ||
				     (j < BoardSize - 1 && CurrentNode.BoardState[i][j + 1] == CurrentNode.Turn))) {

					//Create a moveDS
					node NewNode;
					NewNode.BoardState = CurrentNode.BoardState;
					NewNode.BoardState[i][j] = CurrentNode.Turn;
					NewNode.Move.Row = i;
					NewNode.Move.Column = j;
					NewNode.Move.Type = 1;                              // Raid
					NewNode.Turn = CurrentNode.Turn == 'X' ? 'O' : 'X';
					NewNode.AvailableMoves = CurrentNode.AvailableMoves - 1;

					/**
					 *  Calculate the total Value of neigboring places belonging to the other player
					 *  Capture all neighboring places belonging to the other player
					 *  and store Change list of all changes for reversal at the end
					 */
					int NeighborScore = 0,
							ChangeList_i[4], ChangeList_j[4], ChangeCount = 0;
					if (i > 0 && CurrentNode.BoardState[i - 1][j] != '.' &&
					    CurrentNode.BoardState[i - 1][j] != CurrentNode.Turn) {

						NeighborScore += BoardValue[i - 1][j];

						NewNode.BoardState[i - 1][j] = CurrentNode.Turn;
						ChangeList_i[ChangeCount] = i - 1;
						ChangeList_j[ChangeCount] = j;
						ChangeCount++;
					}
					if (i < BoardSize - 1 && CurrentNode.BoardState[i + 1][j] != '.' &&
					    CurrentNode.BoardState[i + 1][j] != CurrentNode.Turn) {

						NeighborScore += BoardValue[i + 1][j];

						NewNode.BoardState[i + 1][j] = CurrentNode.Turn;
						ChangeList_i[ChangeCount] = i + 1;
						ChangeList_j[ChangeCount] = j;
						ChangeCount++;
					}
					if (j > 0 && CurrentNode.BoardState[i][j - 1] != '.' &&
					    CurrentNode.BoardState[i][j - 1] != CurrentNode.Turn) {

						NeighborScore += BoardValue[i][j - 1];

						NewNode.BoardState[i][j - 1] = CurrentNode.Turn;
						ChangeList_i[ChangeCount] = i;
						ChangeList_j[ChangeCount] = j - 1;
						ChangeCount++;
					}
					if (j < BoardSize - 1 && CurrentNode.BoardState[i][j + 1] != '.' &&
					    CurrentNode.BoardState[i][j + 1] != CurrentNode.Turn) {

						NeighborScore += BoardValue[i][j + 1];

						NewNode.BoardState[i][j + 1] = CurrentNode.Turn;
						ChangeList_i[ChangeCount] = i;
						ChangeList_j[ChangeCount] = j + 1;
						ChangeCount++;
					}


					bool AlphaCut = false, BetaCut = false;

					if (ChangeCount != 0) {

						/**
						 * NeighborScore is removed from the other players tally and added to the current player
						 * Hence the NeighborScore has to be doubled
						 */
						NeighborScore *= 2;

						if (CurrentNode.Turn == YouPlay) {
							NewNode.Score = CurrentNode.Score + BoardValue[i][j] + NeighborScore;
						} else {
							NewNode.Score = CurrentNode.Score - BoardValue[i][j] - NeighborScore;
						}

						//Play the moveDS
						NewNode = step(NewNode, Depth - 1, Alpha, Beta);

						//Max Step
						if (CurrentNode.Turn == YouPlay && NewNode.Score > MaxScore) {
							/**
							 * Max Step
							 */

							MaxScore = NewNode.Score;

							if (MaxScore >= Beta) {
								BetaCut = true;
							}

							Alpha = max(Alpha, MaxScore);

							//Store new best node
							Result = NewNode;

						}//Min Step
						else if (CurrentNode.Turn != YouPlay && NewNode.Score < MinScore) {
							/**
							 * Min Step
							 */

							MinScore = NewNode.Score;

							if (MinScore <= Alpha) {
								AlphaCut = true;
							}

							Beta = min(Beta, MinScore);

							//Store new best node
							Result = NewNode;

						}
					}

					//Reverse moveDS from board for future updates
					CurrentNode.BoardState[i][j] = '.';
					for (int k = 0; k < ChangeCount; k++) {
						CurrentNode.BoardState[ChangeList_i[k]][ChangeList_j[k]] = CurrentNode.Turn == 'X' ? 'O' : 'X';
					}

					//Tree pruning
					if (BetaCut || AlphaCut) {
						CurrentNode.BestNextMove = Result.Move;
						CurrentNode.Score = Result.Score;
						//cout << "---" << endl;
						return CurrentNode;
					}
				}
			}
		}

		//Stake moves
		for (int i = 0; i < BoardSize; i++) {
			for (int j = 0; j < BoardSize; j++) {
				if (CurrentNode.BoardState[i][j] == '.') {

					if (((i > 0 && CurrentNode.BoardState[i - 1][j] == CurrentNode.Turn) ||
					     (i < BoardSize - 1 && CurrentNode.BoardState[i + 1][j] == CurrentNode.Turn) ||
					     (j > 0 && CurrentNode.BoardState[i][j - 1] == CurrentNode.Turn) ||
					     (j < BoardSize - 1 && CurrentNode.BoardState[i][j + 1] == CurrentNode.Turn)) &&
					    ((i > 0 && CurrentNode.BoardState[i - 1][j] != '.' &&
					      CurrentNode.BoardState[i - 1][j] != CurrentNode.Turn) ||
					     (i < BoardSize - 1 && CurrentNode.BoardState[i + 1][j] != '.' &&
					      CurrentNode.BoardState[i + 1][j] != CurrentNode.Turn) ||
					     (j > 0 && CurrentNode.BoardState[i][j - 1] != '.' &&
					      CurrentNode.BoardState[i][j - 1] != CurrentNode.Turn) ||
					     (j < BoardSize - 1 && CurrentNode.BoardState[i][j + 1] != '.' &&
					      CurrentNode.BoardState[i][j + 1] != CurrentNode.Turn))) {
						//Skip moves that can be raided with conquer
						continue;
					}

					//Create a moveDS
					node NewNode;
					NewNode.BoardState = CurrentNode.BoardState;
					NewNode.BoardState[i][j] = CurrentNode.Turn;
					NewNode.Move.Row = i;
					NewNode.Move.Column = j;
					NewNode.Move.Type = 0; // Stake
					NewNode.Turn = CurrentNode.Turn == 'X' ? 'O' : 'X';
					NewNode.AvailableMoves = CurrentNode.AvailableMoves - 1;
					if (CurrentNode.Turn == YouPlay)
						NewNode.Score = CurrentNode.Score + BoardValue[i][j];
					else
						NewNode.Score = CurrentNode.Score - BoardValue[i][j];

					//Play the moveDS
					NewNode = step(NewNode, Depth - 1, Alpha, Beta);

					bool AlphaCut = false, BetaCut = false;

					if (CurrentNode.Turn == YouPlay && NewNode.Score > MaxScore) {
						/**
						 * Max Step
						 */

						MaxScore = NewNode.Score;

						if (MaxScore >= Beta) {
							BetaCut = true;
						}

						Alpha = max(Alpha, MaxScore);

						//Store new best node
						Result = NewNode;

					} else if (CurrentNode.Turn != YouPlay && NewNode.Score < MinScore) {
						/**
						 * Min Step
						 */

						MinScore = NewNode.Score;

						if (MinScore <= Alpha) {
							AlphaCut = true;
						}

						Beta = min(Beta, MinScore);

						//Store new best node
						Result = NewNode;

					}

					//Reverse moveDS from board for future updates
					CurrentNode.BoardState[i][j] = '.';

					//Tree pruning
					if (BetaCut || AlphaCut) {
						CurrentNode.BestNextMove = Result.Move;
						CurrentNode.Score = Result.Score;
						//cout << "---" << endl;
						return CurrentNode;
					}
				}
			}
		}

		CurrentNode.BestNextMove = Result.Move;
		CurrentNode.Score = Result.Score;

		//cout << "---" << endl;
		return CurrentNode;
	}

public:
	Competition(int **BoardValue, int BoardSize, char YouPlay) {
		this->BoardValue = BoardValue;
		this->BoardSize = BoardSize;
		this->YouPlay = YouPlay;
	}

	string run(char **BoardState, int Score, int AvailableMoves, int Depth) {
		node Node;
		Node.BoardState = BoardState;
		Node.Score = Score;
		Node.AvailableMoves = AvailableMoves;
		Node.Turn = YouPlay;

		return print(step(Node, Depth, -INF, INF));
	}
};

int main() {
	int N = 0, Depth = 0, Score = 0, AvailableMoves = 0;
	string Mode, TempString;
	char YouPlay = 'X';
	int **BoardValue = nullptr;
	char **BoardState = nullptr;
	double Time = 0;

	ifstream InputFile("input.txt");
	if (InputFile.is_open()) {
		//Get game board dimensions
		getline(InputFile, TempString);
		N = stoi(TempString);

		//Get game mode
		getline(InputFile, Mode);

		//get player turn
		getline(InputFile, TempString);
		YouPlay = TempString[0];

		//get search depth
		getline(InputFile, TempString);
		if (Mode.compare("COMPETITION") == 0) {
			Time = stod(TempString);
		} else {
			Depth = stoi(TempString);
		}

		//Get BoardValue
		BoardValue = new int *[N];
		for (int i = 0; i < N; i++) {
			BoardValue[i] = new int[N];

			getline(InputFile, TempString);
			stringstream SS(TempString);

			for (int j = 0; j < N; j++)
				SS >> BoardValue[i][j];
		}

		//Get BoardState
		BoardState = new char *[N];
		for (int i = 0; i < N; i++) {
			BoardState[i] = new char[N];

			getline(InputFile, TempString);
			stringstream SS(TempString);

			for (int j = 0; j < N; j++) {
				SS >> BoardState[i][j];
				if (BoardState[i][j] != '.') {
					if (BoardState[i][j] == YouPlay)
						Score += BoardValue[i][j];
					else
						Score -= BoardValue[i][j];
				} else {
					AvailableMoves++;
				}
			}
		}

		InputFile.close();
	} else {
		cout << "Input file failed to load" << endl;
	}

	//Output variable
	string Output = "";

	if (Mode.compare("MINIMAX") == 0) {
		cout << "Running MINIMAX till depth " << Depth << endl;
		MiniMax MM(BoardValue, N, YouPlay);
		Output = MM.run(BoardState, Score, AvailableMoves, Depth);

	} else if (Mode.compare("ALPHABETA") == 0) {
		cout << "Running ALPHABETA till depth " << Depth << endl;
		AlphaBeta AB(BoardValue, N, YouPlay);
		Output = AB.run(BoardState, Score, AvailableMoves, Depth);

	} else if (Mode.compare("COMPETITION") == 0) {
		Depth = getDepth(AvailableMoves, Time);

		cout << "Running COMPETITION till depth " << Depth << endl;
		//Start Clock
		double WallTime = get_wall_time();

		Competition C(BoardValue, N, YouPlay);
		Output = C.run(BoardState, Score, AvailableMoves, Depth);

		//Stop Clock
		WallTime = get_wall_time() - WallTime;

		updateTimePerNode(WallTime, AvailableMoves, Depth);

		cout << "Completed in " << WallTime << " seconds" << endl;
		cout << "Time Remaining : " << Time - WallTime << " seconds" << endl;
		cout << "Moves Remaining : " << AvailableMoves - 1 << endl;
	}

	//cout << Output;

	ofstream OutputFile("output.txt");
	if (OutputFile.is_open()) {
		OutputFile << Output;
		cout << "Output Success" << endl;
		OutputFile.close();
	} else {
		cout << "Output file failed to load" << endl;
	}

	return 0;
}