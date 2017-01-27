agent: homework11.cpp calibration.cpp
	g++ -std=c++11 homework11.cpp -o myagent
	g++ -std=c++11 calibration.cpp -o calibration

run:
	./myagent

calibrate:
	./calibration

newgame:
	/bin/rm -f GameData.txt