/*
	Author : Majd Ewawi (177535)

	Last modified : 12/5/2020

	This is Operating Systems project part 1

	Simulate a CPU Scheduler
	Algorithms : SJF , FCFS , Round Robin scheduling

*/

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <cstdlib>
#include <algorithm>
#include <iomanip>

using namespace std;

// struct represent process contains it's information PCB
// process have : id , arrival time , burst time and size

struct Process {
	int id;
	int burst_time;
	int arrival_time;
	int size;

	 bool operator ==(Process x) {
		return
			x.id == id && x.arrival_time == arrival_time && x.burst_time ==burst_time && x.size == size;
	}
};

// struct represent basic information to perform scheduling

struct BasicInformation {
	int physicalMemorySize;
	int frameSize;
	int roundRobinQuantum;
	int contextSwitch;
};

// struct represent process interval when it's started and finsished

struct ProcessInterval {
	int process_id;
	int starting_time;
	int finishing_time;
};

/******************************************* Read processes form file ****************************************************/

// function that reads process file line by line to get processes data

vector<string> readProcessFile(string fileName) {
	ifstream inFile;

	// open file that contains process information 
	inFile.open("./../../" + fileName);

	// check if the file exists
	if (!inFile) {
		cout << "Unable to open file process.txt";
		exit(1);		 // call system to stop
	}

	string line;
	vector<string> fileDataArray;

	// read all information from process.txt
	while (getline(inFile, line)) {
		fileDataArray.push_back(line);
	}

	// close process file
	inFile.close();

	return fileDataArray;
}

// function that takes string which we read form the file and transform it 
// into struct of Process
// this function return a vector of type Process

vector<Process> createProcesses(string fileName) {

	vector<Process> processInfo;
	vector <string> fileDataArray = readProcessFile(fileName);

	// get the information of each process , split data and put them in vector
	for (int i = 0; i < fileDataArray.size(); i++) {

		// iterate data until reach processes information

		if (fileDataArray.at(i).find("process info:") != string::npos) {
			for (int j = i + 1; j < fileDataArray.size(); j++) {

				// get each line from process file , each line represent process info
				string line = fileDataArray.at(j);

				// split each line to spearte data into
				// process data contains [process id , arrival time ,burst time , size ]

				vector<string> v;
				stringstream ss(line);
				while (ss.good()) {
					string substr;
					getline(ss, substr, ' ');
					v.push_back(substr);
				}

				// transform from string information into numbers

				vector<int> processData;
				for (int i = 0; i < v.size(); i++) {
					stringstream ss(v[i]);
					int number = 0;
					ss >> number;
					processData.push_back(number);
				}

				// prepare process info to add it into the struct
				Process  process;
				process.id = processData[0];
				process.arrival_time = processData[1];
				process.burst_time = processData[2];
				process.size = processData[3];


				// add process data vector into all processes information vector
				processInfo.push_back(process);
			}
		}

	}
	return processInfo;

}
// function that reads basic information form proces file
// this function returns struct of type basic information

BasicInformation readCPUInformation(string fileName) {

	// read process file
	vector<string> fileDataArray = readProcessFile(fileName);
	vector<int> numbers;


	// transform text into integers represent CPU information
	for (int i = 0; i < 4; i++) {
		stringstream ss(fileDataArray.at(i));
		int number;
		ss >> number;
		numbers.push_back(number);
	}

	// create struct represent basic information 
	// assign data into the fields

	BasicInformation info;
	info.physicalMemorySize = numbers.at(0);
	info.frameSize = numbers.at(1);
	info.roundRobinQuantum = numbers.at(2);
	info.contextSwitch = numbers.at(3);

	return info;
}

/****************************** Printing section ********************************************************/

// function that takes processes vector and display them
void printProcessesData(vector<Process> processes) {

	// iterate through processes to print all fields
	cout << "Processes : \n" << endl;
	for (int i = 0; i < processes.size(); i++) {
		cout << "id: " << processes[i].id << " ";
		cout << "arrival time : " << processes[i].arrival_time << " ";
		cout << "burst time : " << processes[i].burst_time << " ";
		cout << "size : " << processes[i].size << " ";
		cout << endl;
	}
	cout << endl;
}

// function that print basic information 

void printCPUData(BasicInformation info) {

	// iterate through struct of basic information read fields
	cout << "Basic information : \n" << endl;
	cout << "Physical memory size : "<<info.physicalMemorySize << endl;
	cout << "Frame size :" <<info.frameSize << endl;
	cout <<"Round Robin Quantum : "<< info.roundRobinQuantum << endl;
	cout << "Context Switch : "<<info.contextSwitch << endl;
}

/********************* This function is used by all scheduling algorithms *****************************************/

// function that calculates total burst time used for calculating CPU utilization

double calculateTotalBurstTime(vector <Process> processes) {
	double totalBurstTime = 0;
	for (int i = 0; i < processes.size(); i++) {
		totalBurstTime += processes.at(i).burst_time;
	}
	return totalBurstTime;
}

// function that calculates cpu utilization 
// cpu utilization = processes total time /(processes total time *context switch* (numberofProcesses-1))

double CPUutilization(vector <ProcessInterval> gantChart ,vector <Process> processes , BasicInformation info) {

	// read context switch value form file 
	int contextSwitch = info.contextSwitch;

	// this value represent number of cycles perform by round robin algorithm
	int numberOfCycles = gantChart.size() - 1;

	// take finishing time for last process
	double totalBurstTime = calculateTotalBurstTime(processes);

	double cpuUtilization =totalBurstTime / (totalBurstTime  + (contextSwitch * (numberOfCycles - 1)));

	return cpuUtilization;

}
/************************************** SJF , FCFS section *********************************************************/


// function used to compare between processes according to the arrival time
// this function is used in sort vector descinding

bool sortByArrivalTimeDESC(Process process1 , Process process2){
	return process1.arrival_time < process2.arrival_time; 
}


// function used to apply FCFS algorithm which serve processes according to the arrival time
// takes vector of processes arange them and return vector

vector < ProcessInterval> FCFS(vector<Process> processes, BasicInformation info) {

	vector <ProcessInterval> processesFCFS;

	int currentTime = 0;

	// sort vector by arrival time
	sort(processes.begin(), processes.end(), sortByArrivalTimeDESC);

	while (true) {

		// iterate through all processes
		//  if the process is arrived we serve it  
		// update current time remove served process from the temporarily vector

		for (int i = 0; i < processes.size(); i++) {
			bool found = false;
			if (currentTime >= processes.at(i).arrival_time) {

				for (int j = 0; j < processesFCFS.size(); j++) {
					if (processes.at(i).id == processesFCFS.at(j).process_id)
						found = true;
				}

				if (found == true) {
					continue;
				}
				else {
					ProcessInterval item;
					item.starting_time = currentTime;
					item.process_id = processes.at(i).id;
					currentTime += processes.at(i).burst_time;
					item.finishing_time = currentTime;

					processesFCFS.push_back(item);

					currentTime += info.contextSwitch;

				}
			}
			else {
				currentTime++;
			}
		}

		if (processes.size() == processesFCFS.size())
			break;
	}


	
	return processesFCFS;

}

// function used to compare between processes according to the burst time
// this function is used in sort vector descinding

bool sortByBurstTimeDESC(Process process1, Process process2) {
	return process1.burst_time < process2.burst_time;


}


// function used for create arrange processes according SJF scheduling algorithm 
// SJF non preemtive
// shortest process is served first take in consideration arrival time
// this function takes processes as argument and arange them return the vector after 
// applying SJF algorithm

vector <ProcessInterval> SJF(vector<Process> processes ,  BasicInformation info) {

	vector <ProcessInterval> processesSJF;

	int currentTime = 0;

	// define first process to start [process with smallest arrival time]

	ProcessInterval item;

	item.starting_time = currentTime;

	Process firstProcess = *min_element(processes.begin(), processes.end(), sortByArrivalTimeDESC);
	item.process_id = firstProcess.id;

	// update current time , this is used to know which process hasn't arrived yet

	currentTime += firstProcess.arrival_time + firstProcess.burst_time ;

	item.finishing_time = currentTime;

	// adding first process to the gant chart
	processesSJF.push_back(item);

	// update current time adding context swich time
	currentTime += info.contextSwitch;

	// sort vector by burst time and arrival time
	sort(processes.begin(), processes.end(), sortByBurstTimeDESC);
	sort(processes.begin(), processes.end(), sortByArrivalTimeDESC);

	while (true) {
		bool finished = false;

		// iterate through all processes
		//  if the process is arrived we serve it  
		// update current time remove served process from the temporarily vector

		for (int i = 0; i < processes.size(); i++) {
			bool found = false;
			if (currentTime >= processes.at(i).arrival_time) {

				for (int j = 0; j < processesSJF.size(); j++) {
					if (processes.at(i).id == processesSJF.at(j).process_id)
						found = true;
				}

				if (found == true) {
					continue;
				}
				else {
					ProcessInterval item;
					item.starting_time = currentTime;
					item.process_id = processes.at(i).id;
					currentTime += processes.at(i).burst_time;
					item.finishing_time = currentTime;

					processesSJF.push_back(item);

					currentTime += info.contextSwitch;

				}
			}
			else {
				currentTime++;
			}
		}

		if (processes.size() == processesSJF.size())
			break;
	}

	

	return processesSJF;
}

// function to find process form it's interval

Process findProcess(ProcessInterval processInterval , vector <Process> processes) {
	Process process = {0,0,0};
	for (int i = 0; i < processes.size(); i++) {
		if (processes.at(i).id == processInterval.process_id)
			return processes.at(i);
	}
	return process;
}
// function that calculates Waiting time for each process after scheduling processes in SJF and FCFS 
// waiting time = starting time for process - arrival time

vector <int> processesWaitingTime(vector <Process> processes , vector<ProcessInterval> processesInterval) {
	vector <int> processesWaitingTime;

	for (int i = 0; i < processesInterval.size(); i++) {
		Process process = findProcess(processesInterval.at(i), processes);
		int waitingTime = processesInterval.at(i).starting_time - process.arrival_time;
		processesWaitingTime.push_back(waitingTime);
	}

	return processesWaitingTime;
}


// function that calculates turnaround time for each process after scheduling processes in SJF FCFS
// turnaroundtime = finishing time - arrival time

vector <int> processesTurnaroundTime(vector<Process> processes, vector <ProcessInterval> processesInterval) {
	vector <int> processesTurnaroundTime;

	for (int i = 0; i < processes.size(); i++) {
		Process process = findProcess(processesInterval.at(i), processes);
		int turnaroundTime = processesInterval.at(i).finishing_time - process.arrival_time;
		processesTurnaroundTime.push_back(turnaroundTime);
	}

	return processesTurnaroundTime;
}

// function that calculates average waiting time for all processes
// function that takes vector contains waitng for each process 
// and calculate average waiting time

double findAverageWaitingTime(vector <int> processesWaitingTime) {

	double averageWaitingTime = 0;

	for (int i = 0; i < processesWaitingTime.size(); i++) {

		averageWaitingTime += processesWaitingTime.at(i);
	}
	return averageWaitingTime / processesWaitingTime.size();
}

// function that calculates average finishing time for all processes
// function that takes vector contains finishing for each process 
// and calculate average finishing time

double findAverageFinishingTime(vector <ProcessInterval> processesIntervals) {
	double averageFinishingTime = 0;

	for (int i = 0; i < processesIntervals.size(); i++) {

		averageFinishingTime += processesIntervals.at(i).finishing_time;
	}
	return averageFinishingTime / processesIntervals.size();
}


// function that calculates average turnaround time for all processes
// function that takes vector contains turnaround for each process 
// and calculate average turnaround time

double findAverageTurnaroundTime(vector <int> processesTurnaroundTime) {

	double averageTurnaroundTime = 0;

	for (int i = 0; i < processesTurnaroundTime.size(); i++) {
		averageTurnaroundTime += processesTurnaroundTime.at(i);
	}
	return averageTurnaroundTime / processesTurnaroundTime.size();
}



// function that takes processes after scheduling and generate gant chart for FCFS and SJF

void generateSchedulingData(vector <Process> processes , vector<ProcessInterval> gantChart , BasicInformation info) {

	// functions calls for data generated after scheduling

	vector <int> waitingTime = processesWaitingTime(processes, gantChart);
	vector <int> turnaroundTime = processesTurnaroundTime(processes, gantChart);

	double averageWaitingTime = findAverageWaitingTime(waitingTime);
	double averageTurnaroundTime = findAverageTurnaroundTime(turnaroundTime);
	double averageFinishingTime = findAverageFinishingTime(gantChart);
	double cpuUtilization = CPUutilization(gantChart , processes , info);

	// print all information about process after apply scheduling algorithmm

	for (int i = 0; i < gantChart.size(); i++) {
		cout << "Process :" << gantChart.at(i).process_id << " ";
		cout << "Started :" << gantChart.at(i).starting_time << " ";
		cout << "Finished :" << gantChart.at(i).finishing_time << " ";
		cout << " Waiting Time : " << waitingTime.at(i) << " ";
		cout << " Turnaround Time : " << turnaroundTime.at(i) << " ";
		cout << endl;
	}

	cout << endl;
	cout << "Average waiting time = " << averageWaitingTime << endl;
	cout << "Average turnaround time = " << averageTurnaroundTime << endl;
	cout << "Average Finishing time = " << averageFinishingTime << endl;
	cout << "CPU Utilization time = " << cpuUtilization << endl;
}



/***********************************************Round Robin Scheduling part ************************************/

// function that calculates waiting time for each process in  round robin scheduling
// this function takes processes after apply round robin on them , 
// and takes processesInterval which represnt finishing , and starting time for each process

int* calculateWaitingTimeRoundRobin(vector <Process> processes ,vector <ProcessInterval> processesInterval,string fileName) {
	int numberOfProcesses = createProcesses(fileName).size();
	int* waitingTime = new int[numberOfProcesses];

	double sumOfWaitingTime=0;

	for (int i = 0; i < processesInterval.size(); i++) {
		int index = processesInterval.at(i).process_id;
		Process process = findProcess(processesInterval.at(i), processes);
		waitingTime[index] = processesInterval.at(i).starting_time - process.arrival_time;
		for (int j = i + 1; j < processesInterval.size(); j++) {

			// check  if the same process shows again after time
			if (processesInterval.at(i).process_id == processesInterval.at(j).process_id) {

				// waiting time for any process round robin 
				// = waitingtime for first time + starting time for second time - finshing time for first time

				waitingTime[index] += processesInterval.at(j).starting_time - processesInterval.at(i).finishing_time;
				processesInterval.erase(processesInterval.begin() + j);
			}
		}
		
	}
	return waitingTime;
}

// function that calculates finishing time for each process in  round robin scheduling
// this function takes processes after apply round robin on them , 
// and takes processesInterval which represnt finishing , and starting time for each process

int* calculateFinishingTimeRoundRobin(vector <Process> processes, vector <ProcessInterval> processesInterval , string fileName) {

	int numberOfProcesses = createProcesses(fileName).size();
	int* finishingTime = new int[numberOfProcesses];

	double sumOfFinishingTime = 0;

	for (int i = 0; i < processesInterval.size(); i++) {
		
		int index = processesInterval.at(i).process_id;
		finishingTime[index] = processesInterval.at(i).finishing_time ;
		for (int j = i + 1; j < processesInterval.size(); j++) {

			// check if the process showes agin in the gant chart

			if (processesInterval.at(i).process_id == processesInterval.at(j).process_id) {

				// finishing time for process in round robin = last finishing time

				finishingTime[index] = processesInterval.at(i).finishing_time;
			}
		}

	}

	return finishingTime;
}

// function that calculates turnaround time for each process in  round robin scheduling
// this function takes processes after apply round robin on them , 
// and takes processesInterval which represnt finishing , and starting time for each process

int*  calculateTurnaroundTimeRoundRobin(vector <Process> processes, vector <ProcessInterval> processesInterval , string fileName) {

	int numberOfProcesses = createProcesses(fileName).size();

	int* turnaroundTime = new int[numberOfProcesses];
	int* finishingTime = new int[numberOfProcesses];
	

	double sumOfTurnaroundTime = 0;

	for (int i = 0; i < processesInterval.size(); i++) {
		int index = processesInterval.at(i).process_id;

		Process process = findProcess(processesInterval.at(i), processes);
		finishingTime[index] = processesInterval.at(i).finishing_time;

		for (int j = i + 1; j < processesInterval.size(); j++) {
			if (processesInterval.at(i).process_id == processesInterval.at(j).process_id) {

				// turnaround time = last finishing time - arrival time

				finishingTime[index] = processesInterval.at(i).finishing_time;
			}
		}
		turnaroundTime[index] = finishingTime[index] - process.arrival_time;
	}

	cout << endl;

	return turnaroundTime;

}

// function that takes vector of processes and apply round robin algorithm 
// this function return processes data after scheduling define process id when it's starts 
// and it's finishing time

void generateDataRoundRobin(vector <Process> processes ,vector <ProcessInterval> gantChart , string fileName) {

	BasicInformation info = readCPUInformation(fileName);

	// number of processes = process we read from file

	int numberOfProcesses = createProcesses(fileName).size();


	for (int i = 0; i < gantChart.size(); i++) {
		cout << "Process:" << gantChart.at(i).process_id << " ";
		cout << "Starts at:" << gantChart.at(i).starting_time << " ";
		cout << "Finished at:" << gantChart.at(i).finishing_time << " ";

		cout << endl;
	}

	double cpuUtilization = CPUutilization(gantChart , createProcesses(fileName) , info);

	int * finishingTime =calculateFinishingTimeRoundRobin(processes, gantChart, fileName);
	int* waitingTime = calculateWaitingTimeRoundRobin(processes, gantChart , fileName);
	int* turnaroundTime = calculateTurnaroundTimeRoundRobin(processes, gantChart,fileName);
	
	double sumOfTurnaroundTime = 0;
	double sumOfWaitingTime = 0;
	double sumOfFinishingTime = 0;

	for (int i = 0; i < numberOfProcesses; i++) {
		cout << "Process :" << i << " Turnaround time :" << turnaroundTime[i] ;
		cout << " Waiting time :" << waitingTime[i] << " ";
		cout << " Finsihing time :" << finishingTime[i] << endl;
		sumOfTurnaroundTime += turnaroundTime[i];
		sumOfWaitingTime += turnaroundTime[i];
		sumOfFinishingTime += turnaroundTime[i];
	}

	cout << "\nAverage Turnaround time : " << sumOfTurnaroundTime / 5 << endl;
	cout << "Average Waiting time : " << sumOfWaitingTime / 5 << endl;
	cout << "Average Finishing time : " << sumOfFinishingTime / 5 << endl;

	cout << "CPU Utilization time = " << cpuUtilization << endl;

	cout << endl;
}

// function to apply round robin scheduling and calculate waiting time 

vector<ProcessInterval> roundRobinScheduling(vector <Process> processes , string fileName) {

	BasicInformation info = readCPUInformation(fileName);
	int roundRobinQuantum = info.roundRobinQuantum;

	vector <ProcessInterval> gantChart;

	vector <int> remainingTime;


	// create temporarily vector of integer represent remaining time for each process
	// at the begining remaining time = burst time for each process

	for (int i = 0; i < processes.size(); i++) {
		remainingTime.push_back(processes.at(i).burst_time);
	}

	int currentTime = 0; 

// Keep traversing processes in round robin manner 
// until all of them are not done. 
	while (true){
		
		bool done = true;

		// Traverse all processes one by one repeatedly 
		for (int i = 0; i < processes.size(); i++) {

			if (processes.at(i).arrival_time > currentTime)
				continue;
			else {
		
				// If burst time of a process is greater than 0 
				// then only need to process further

				if (remainingTime.at(i) > 0) {
					done = false; // There is a pending process 

					ProcessInterval item;

					if (remainingTime.at(i) > roundRobinQuantum)
					{
					
						// Increase the value of t i.e. shows 
						// how much time a process has been processed 

						// create gant chart item
						item.starting_time = currentTime;
						item.process_id = processes.at(i).id;

						currentTime += roundRobinQuantum;

						item.finishing_time = currentTime; // prosess finished after quantum

						gantChart.push_back(item);

						// Decrease the burst_time of current process 
						// by quantum 
						remainingTime.at(i) -= roundRobinQuantum;
					}

					// If burst time is smaller than or equal to 
					// quantum. Last cycle for this process 
					else
					{
						// Increase the value of t i.e. shows 
						// how much time a process has been processed 

						// create gant chart item
						item.starting_time = currentTime;
						item.process_id = processes.at(i).id;

						currentTime += remainingTime.at(i);

						item.finishing_time = currentTime;

						gantChart.push_back(item);

						// As the process gets fully executed 
						// make its remaining burst time = 0 
						remainingTime.at(i) = 0;
					}
				}

			}
		}

		// If all processes are done 
		if (done == true)
			break;
	}
	
	return gantChart;
}

int main() {

	// variables declaration
	string fileName = "process.txt";

	// read processes from file return vector of processes to apply algorithms

	vector<Process> processes = createProcesses(fileName);
	BasicInformation info = readCPUInformation(fileName);


	// print data we read form the file

	cout << endl;

	printCPUData(info);

	cout << endl;

	printProcessesData(processes);


	// print processes after applying scheduling algorithms
	// print waiting time , turnaround time , finishing , starting time for each process
	// average finishing time , average turnaround time , average finishing time
	// CPU utilization for each scheduling algorithm

	cout << "FCFS Scheduling : \n" << endl;
	vector<ProcessInterval> gantChartFCFS = FCFS(processes , info);
	generateSchedulingData(processes, gantChartFCFS , info);
	cout << endl;


	cout << "SJF Scheduling : \n" << endl;
	// generate data for each process when the process starts and ends
	vector <ProcessInterval> gantChartSJF = SJF(processes, info);
	generateSchedulingData(processes ,gantChartSJF, info);
	cout << endl;

	cout << "Round Robin Scheduling : \n" << endl;
	vector<ProcessInterval> gantChartRoundRobin = roundRobinScheduling(processes , fileName);
	generateDataRoundRobin(processes , gantChartRoundRobin, fileName);
	cout << endl;
	 
	system("pause");
	return 0;
}