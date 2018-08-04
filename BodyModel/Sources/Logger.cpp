#include "pch.h"

#include "Logger.h"

#include <Kore/Log.h>

#include <ctime>

Logger::Logger() : initPositionData(false), initTransRotData(false), initEvaluationData(false) {
	time_t t = time(0);   // Get time now
	positionDataPath << positionData << "_" << t << ".csv";
	initTransRotPath << initTransRotFilename << "_" << t << ".csv";
	evaluationDataPath << evaluationDataFilename << "_" << t << ".csv";
	evaluationConfigPath << evaluationConfigFilename << "_" << t << ".csv";
}

Logger::~Logger() {
	positionDataOutputFile.close();
	evaluationDataOutputFile.close();
}

void Logger::saveData(Kore::vec3 rawPos, Kore::Quaternion rawRot) {
	if (!initPositionData) {
		positionDataOutputFile.open(positionDataPath.str(), std::ios::app); // Append to the end
		positionDataOutputFile << "rawPosX;rawPosY;rawPosZ;rawRotX;rawRotY;rawRotZ;rawRotW\n";
		positionDataOutputFile.flush();
		initPositionData = true;
	}
	
	// Save positional and rotation data
	positionDataOutputFile << rawPos.x() << ";" << rawPos.y() << ";" << rawPos.z() << ";" << rawRot.x << ";" << rawRot.y << ";" << rawRot.z << ";" << rawRot.w << "\n";
	positionDataOutputFile.flush();
}

void Logger::saveInitTransAndRot(Kore::vec3 initPos, Kore::Quaternion initRot) {
	if (!initTransRotData) {
		initTransRotDataOutputFile.open(initTransRotPath.str(), std::ios::app);
		initTransRotDataOutputFile << "initPosX;initPosY;initPosZ;initRotX;initRotY;initRotZ;initRotW\n";
		initTransRotDataOutputFile.flush();
		initTransRotData = true;
	}
	
	// Save initial position rotation
	initTransRotDataOutputFile << initPos.x() << ";" << initPos.y() << ";" << initPos.z() << ";" << initRot.x << ";" << initRot.y << ";" << initRot.z << ";" << initRot.w << "\n";
	initTransRotDataOutputFile.flush();
	initTransRotDataOutputFile.close();
}

void Logger::saveEvaluationData(Avatar *avatar) {
	if (!initEvaluationData) {
		evaluationConfigOutputFile.open(evaluationConfigPath.str(), std::ios::app);
		evaluationConfigOutputFile << "IK Mode;with Orientation;File;lambda;Error Pos Max;Error Rot Max;Steps Max\n";
		evaluationConfigOutputFile << ikMode << ";" << withOrientation << ";" << currentFile->positionDataFilename << ";" << lambda[ikMode] << ";"  << errorPosMax << ";"  << errorRotMax << ";"  << maxSteps << ";" << "\n";
		evaluationConfigOutputFile.flush();
		evaluationConfigOutputFile.close();
		
		evaluationDataOutputFile.open(evaluationDataPath.str(), std::ios::app);
		evaluationDataOutputFile << "Iterations;Error Pos;Error Rot;Time;Time/Iteration;";
		evaluationDataOutputFile << "Iterations Min;Error Pos Min;Error Rot Min;Time Min;Time/Iteration Min;";
		evaluationDataOutputFile << "Iterations Max;Error Pos Max;Error Rot Max;Time Max;Time/Iteration Max;";
		evaluationDataOutputFile << "Reached [%];\n";
		evaluationDataOutputFile.flush();
		
		initEvaluationData = true;
	}
	
	float* iterations = avatar->getIterations();
	float* errorPos = avatar->getErrorPos();
	float* errorRot = avatar->getErrorRot();
	float* time = avatar->getTime();
	float* timeIteration = avatar->getTimeIteration();
	
	// Save datas
	for (int i = 0; i < 3; ++i) {
		evaluationDataOutputFile << *(iterations + i) << ";";
		evaluationDataOutputFile << *(errorPos + i) << ";";
		evaluationDataOutputFile << *(errorRot + i) << ";";
		evaluationDataOutputFile << *(time + i) << ";";
		evaluationDataOutputFile << *(timeIteration + i) << ";";
	}
	evaluationDataOutputFile << avatar->getReached() << ";" << "\n";
	evaluationDataOutputFile.flush();
	
	avatar->resetStats();
}

bool Logger::readLine(std::string str, Kore::vec3* rawPos, Kore::Quaternion* rawRot) {
	int column = 0;
	
	if (std::getline(positionDataInputFile, str, '\n')) {
		std::stringstream ss;
		ss.str(str);
		std::string item;
		
		while(std::getline(ss, item, ';')) {
			float num = std::stof(item);
			
			if (column == 0) rawPos->x() = num;
			else if (column == 1) rawPos->y() = num;
			else if (column == 2) rawPos->z() = num;
			else if (column == 3) rawRot->x = num;
			else if (column == 4) rawRot->y = num;
			else if (column == 5) rawRot->z = num;
			else if (column == 6) rawRot->w = num;
			
			++column;
		}
		
		return true;
	}
	
	return false;
}

bool Logger::readData(int line, const int numOfEndEffectors, const char* filename, Kore::vec3* rawPos, Kore::Quaternion* rawRot) {
	std::string str;
	bool success = false;
	
	if (!positionDataInputFile.is_open())
		positionDataInputFile.open(filename);
	
	// Skip lines
	while(line > currLineNumber - 1) {
		std::getline(positionDataInputFile, str, '\n');
		++currLineNumber;
	}
	
	// Read line
	for (int i = 0; i < numOfEndEffectors; ++i) {
		rawPos[i] = Kore::vec3(0, 0, 0);
		rawRot[i] = Kore::Quaternion(0, 0, 0, 1);
		success = readLine(str, &rawPos[i], &rawRot[i]);
		if (success) ++currLineNumber;
	}
	
	if (positionDataInputFile.eof()) {
		positionDataInputFile.close();
		currLineNumber = 0;
	}
	
	return success;
}

void Logger::readInitTransAndRot(const char* filename, Kore::vec3* initPos, Kore::Quaternion* initRot) {
	std::fstream inputFile(filename);
	
	// Get header
	int column = 0;
	std::string str;
	std::getline(inputFile, str, '\n');
	
	// Get initial positional vector and rotation
	std::getline(inputFile, str, '\n');
	std::stringstream ss;
	ss.str(str);
	std::string item;
	while(std::getline(ss, item, ';')) {
		float num = std::stof(item);
		
		//log(Kore::Info, "%i -> %f", column, num);
		
		if (column == 0) initPos->x() = num;
		else if (column == 1) initPos->y() = num;
		else if (column == 2) initPos->z() = num;
		else if (column == 3) initRot->x = num;
		else if (column == 4) initRot->y = num;
		else if (column == 5) initRot->z = num;
		else if (column == 6) initRot->w = num;
		
		++column;
	}
}
