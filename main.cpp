#include "process.h"
#include "ioModule.h"
#include "processMgmt.h"

#include <chrono> // for sleep
#include <thread> // for sleep

int main(int argc, char* argv[])
{
    // single thread processor
    // it's either processing something or it's not
//    bool processorAvailable = true;

    // vector of processes, processes will appear here when they are created by
    // the ProcessMgmt object (in other words, automatically at the appropriate time)
    list<Process> processList;

    // this will orchestrate process creation in our system, it will add processes to
    // processList when they are created and ready to be run/managed
    ProcessManagement processMgmt(processList);

    // this is where interrupts will appear when the ioModule detects that an IO operation is complete
    list<IOInterrupt> interrupts;

    // this manages io operations and will raise interrupts to signal io completion
    IOModule ioModule(interrupts);

    // Do not touch
    long time = 1;
    long sleepDuration = 50;
    string file;
    stringstream ss;
    enum stepActionEnum {noAct, admitNewProc, handleInterrupt, beginRun, continueRun, ioRequest, complete} stepAction;

    // Do not touch
    switch(argc)
    {
        case 1:
            file = "./procList.txt";  // default input file
            break;
        case 2:
            file = argv[1];         // file given from command line
            break;
        case 3:
            file = argv[1];         // file given
            ss.str(argv[2]);        // sleep duration given
            ss >> sleepDuration;
            break;
        default:
            cerr << "incorrect number of command line arguments" << endl;
            cout << "usage: " << argv[0] << " [file] [sleepDuration]" << endl;
            return 1;
            break;
    }

    processMgmt.readProcessFile(file);


    time = 0;
//    processorAvailable = true;

    processMgmt.activateProcesses(time);
    list<Process>::iterator selectedProcess = processList.begin();

    //keep running the loop until all processes have been added and have run to completion
    while(processMgmt.moreProcessesComing() || true /* something might go here */)
    {
        //Update our current time step
        ++time;

        //let new processes in if there are any
        processMgmt.activateProcesses(time);

        //update the status for any active IO requests
        ioModule.ioProcessing(time);

        //If the processor is tied up running a process, then continue running it until it is done or blocks
        //   note: be sure to check for things that should happen as the process continues to run (io, completion...)
        //If the processor is free then you can choose the appropriate action to take, the choices (in order of precedence) are:
        // - admit a new process if one is ready (i.e., take a 'newArrival' process and put them in the 'ready' state)
        // - address an interrupt if there are any pending (i.e., update the state of a blocked process whose IO operation is complete)
        // - start processing a ready process if there are any ready


        //init the stepAction, update below
        stepAction = noAct;


        //TODO add in the code to take an appropriate action for this time step!
        //you should set the action variable based on what you do this time step. you can just copy and paste the lines below and uncomment them, if you want.
        //stepAction = continueRun;  //runnning process is still running
        //stepAction = ioRequest;  //running process issued an io request
        //stepAction = complete;   //running process is finished
        //stepAction = admitNewProc;   //admit a new process into 'ready'
        //stepAction = handleInterrupt;   //handle an interrupt
        //stepAction = beginRun;   //start running a process



        //   <your code here>



        // If a process is currently running, cannot switch task, TOP PRIORITY
        if (selectedProcess->state == processing)
        {
          // Check for interrupts
          if (!interrupts.empty())
          {
            stepAction = handleInterrupt;
            cout << "ID: " << interrupts.front().ioEventID << endl;
            cout << "ID: " << interrupts.front().procID << endl;
            interrupts.pop_front();
          }
          // Check if process needs to request IO event
          else if (selectedProcess->processorTime == selectedProcess->ioEvents.begin()->time)
          {
            ioModule.submitIORequest(time, selectedProcess->ioEvents.front(), *selectedProcess);

            selectedProcess->state = blocked;
            stepAction = ioRequest;
          }
          // Otherwise Process normally
          else if (selectedProcess->processorTime < selectedProcess->reqProcessorTime)
          {
            selectedProcess->processorTime++;
            stepAction = continueRun;
          }
          // Once finished end task
          else
          {
            selectedProcess->state = done;
            selectedProcess->doneTime = time;
            stepAction = complete;
          }
        }
        else
        {
          selectedProcess++;
          bool checkingProcesses = true;

          while (checkingProcesses)
          {
            switch(selectedProcess->state)
            {
              case processing:

                // You shouldn't be here O_O
                // Processing handled above
                break;

              case ready:

                selectedProcess->state = processing;
                selectedProcess->processorTime++;
                stepAction = beginRun;

                checkingProcesses = false;
                break;

              case blocked:

                // Ignore this process
                selectedProcess++;
                break;

              case newArrival:

                selectedProcess->state = ready;
                stepAction = admitNewProc;

                checkingProcesses = false;
                break;

              case done:

                // Ignore this process
                selectedProcess++;
                break;
            }

            // Wrap pointer back around
            if (selectedProcess == processList.end())
            {
              selectedProcess = processList.begin();
            }
          }
        }



        // Leave the below alone (at least for final submission, we are counting on the output being in expected format)
        cout << setw(5) << time << "\t";

        switch(stepAction)
        {
            case admitNewProc:
              cout << "[  admit]\t";
              break;
            case handleInterrupt:
              cout << "[ inrtpt]\t";
              break;
            case beginRun:
              cout << "[  begin]\t";
              break;
            case continueRun:
              cout << "[contRun]\t";
              break;
            case ioRequest:
              cout << "[  ioReq]\t";
              break;
            case complete:
              cout << "[ finish]\t";
              break;
            case noAct:
              cout << "[*noAct*]\t";
              break;
        }

        // You may wish to use a second vector of processes (you don't need to, but you can)
        printProcessStates(processList); // change processList to another vector of processes if desired


        this_thread::sleep_for(chrono::milliseconds(sleepDuration));
    }

    return 0;
}
