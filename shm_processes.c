#include  <stdio.h>
#include  <stdlib.h>
#include  <sys/types.h>
#include  <sys/ipc.h>
#include  <sys/shm.h>
#include <unistd.h>
#include <sys/wait.h>

void  ChildProcess(int  SharedMem[])
{
     int i, account, randomVal;
     srand(getpid());
  
     for(i = 0; i < 25; i++){
       sleep(rand() % 6);
       account = SharedMem[0];
       
       while (SharedMem[1] != 1);

       randomVal = rand() % 51;
       printf("Poor Student needs $%d\n", randomVal);
       
       if (SharedMem[0] <= account) {
         account -= SharedMem[0];
         printf("Poor Student: Withdraws $%d / Balance = $%d\n", SharedMem[0], account);
         SharedMem[0] = account;
       }
       else {
         printf("Poor Student: Not Enough Cash ($%d)\n", account );
       }
       SharedMem[1] = 0; // set turn to 0

     }
}

void  ParentProcess(int  SharedMem[])
{
  int i, account, randomVal;
  srand(getpid()); // seed the pseudo rand num generator
  
  for(i = 0; i < 25; i++) {
    // #1 sleep from 0 to 5
    sleep(rand() % 6);
    account = SharedMem[0];
    
    while(SharedMem[1] != 0);
    
    if (account <= 100) { // deposit money
      randomVal = rand() % 101; // generate num between 0-100
      
      if (randomVal % 2 == 0) { //if even
        account += randomVal;
        
        printf("Dear old Dad: Deposits $%d / Balance = $%d\n", SharedMem[0], account);
        
      }
      else {
        printf("Dear old Dad: Doesn't have any money to give\n");
      }
      SharedMem[0] = account; // copy local account to bank
      SharedMem[1] = 1; // set turn - 1
    } 
    else  {
      printf("Dear old Dad: Thinks Student has enough Cash ($%d)\n", account);
    }
  }
}

int  main(int  argc, char *argv[])
{
     int    ShmID;
     int    *ShmPTR;
     pid_t  pid;
     int    status;

     ShmID = shmget(IPC_PRIVATE, 2*sizeof(int), IPC_CREAT | 0666);
     if (ShmID < 0) {
          printf("*** shmget error (server) ***\n");
          exit(1);
     }
     printf("Server has received a shared memory of four integers...\n");

     ShmPTR = (int *) shmat(ShmID, NULL, 0);
     if (*ShmPTR == -1) {
          printf("*** shmat error (server) ***\n");
          exit(1);
     }
     printf("Server has attached the shared memory...\n");
  
     ShmPTR[0] = 0; // bank account
     ShmPTR[1] = 0; // turn
    
     //printf("Main has filled bank account = %d turn = %d in shared memory...\n",
     //      ShmPTR[0], ShmPTR[1], ShmPTR[2], ShmPTR[3]);

     printf("Main is about to fork a child process...\n");
     pid = fork();
     if (pid < 0) {
          printf("*** fork error (Main) ***\n");
          exit(1);
     }
     else if (pid == 0) { // poor student
          ChildProcess(ShmPTR);
          exit(0);
     }
     else {
          ParentProcess(ShmPTR);
     }


     wait(&status);
     printf("Main has detected the completion of its child...\n");
     shmdt((void *) ShmPTR);
     printf("Main has detached its shared memory...\n");
     shmctl(ShmID, IPC_RMID, NULL);
     printf("Main has removed its shared memory...\n");
     printf("Main exits...\n");
     exit(0);
}
